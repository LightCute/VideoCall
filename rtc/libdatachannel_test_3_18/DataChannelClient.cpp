#include "DataChannelClient.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <thread>


using namespace std::chrono_literals;
using nlohmann::json;

DataChannelClient::DataChannelClient() {
    Log::init("app.log", Log::Mode::Async, spdlog::level::trace);
    Log::info("[DataChannelClient] Starting, thread id: [{}]", Log::threadIdToString(std::this_thread::get_id()));
    rtc::InitLogger(rtc::LogLevel::Debug, myCppLogCallback);
    m_localId = randomId(4);
    Log::info("[DataChannelClient] Local ID generated: {}", m_localId);
    m_config.iceServers.emplace_back("stun:stun.l.google.com:19302");
}

DataChannelClient::~DataChannelClient() {
    close();
}

void DataChannelClient::connectToServer(const std::string& serverUrl) {
    if (m_ws && m_ws->isOpen()) {
        Log::warn("[WebSocket] WebSocket is already connected");
        return;
    }

    m_ws = std::make_shared<rtc::WebSocket>();
    m_wsPromise = std::promise<void>();
    auto wsFuture = m_wsPromise.get_future();

    m_ws->onOpen([this]() {
        Log::info("[WebSocket] WebSocket connected, signaling ready");
        m_wsPromise.set_value();
    });

    m_ws->onError([this](std::string s) {
        Log::error("[WebSocket] WebSocket error: {}", s);
        m_wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    m_ws->onClosed([this]() {
        Log::info("[WebSocket] WebSocket closed");
    });

    m_ws->onMessage([this, wws = std::weak_ptr<rtc::WebSocket>(m_ws)](auto data) {
        if (!std::holds_alternative<std::string>(data)) return;

        json message = json::parse(std::get<std::string>(data));
        auto it = message.find("id");
        if (it == message.end()) return;
        auto id = it->get<std::string>();

        it = message.find("type");
        if (it == message.end()) return;
        auto type = it->get<std::string>();

        std::shared_ptr<rtc::PeerConnection> pc;
        if (auto jt = m_clients.find(id); jt != m_clients.end()) {
            pc = jt->second->peerConnection;
        } else if (type == "offer") {
            Log::info("[WebSocket] Answering to {}", id);
            pc = createPeerConnection(wws, id);
        } else {
            return;
        }
        if (type == "offer" || type == "answer") {
            auto sdp = message["description"].get<std::string>();
            pc->setRemoteDescription(rtc::Description(sdp, type));
        } else if (type == "candidate") {
            auto sdp = message["candidate"].get<std::string>();
            auto mid = message["mid"].get<std::string>();
            pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
        }
    });

    std::string fullUrl = serverUrl + "/" + m_localId;
    m_ws->open(fullUrl);

    Log::info("[WebSocket] Waiting for signaling connection...");
    wsFuture.get();
    Log::info("[WebSocket] Waiting for signaling connected...");
}

void DataChannelClient::callPeer(const std::string& peerId) {
    if (!m_ws || !m_ws->isOpen()) {
        Log::error("[DataChannelClient] WebSocket is not connected");
        return;
    }

    if (peerId.empty() || peerId == m_localId) {
        Log::warn("[DataChannelClient] Invalid peer ID");
        return;
    }

    m_connectionPromisePtr = std::make_shared<std::promise<void>>();
    m_connectedFlag = false;
    Log::info("[DataChannelClient] Offering to {}", peerId);
    auto pc = createPeerConnection(m_ws, peerId);

    auto video = rtc::Description::Video("video-stream", rtc::Description::Direction::SendRecv);
    auto track = pc->addTrack(video);

    const std::string label = "test";
    Log::info("[DataChannelClient] Creating DataChannel with label \"{}\"", label);
    auto dc = pc->createDataChannel(label);



    dc->onOpen([this, peerId, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
        Log::info("[DataChannelClient] DataChannel from {} open", peerId);
        m_connectedFlag = true;
        if (m_connectionPromisePtr) {
            try {
                m_connectionPromisePtr->set_value();
            } catch (const std::future_error& e) {
                Log::warn("[DataChannelClient] Promise already set: {}", e.what());
            }
        }
        
        if (m_onConnectedCallback) m_onConnectedCallback(peerId);
        if (auto dc = wdc.lock()) dc->send("Hello from " + m_localId);
    });

    dc->onClosed([this, peerId]() {
        Log::info("[DataChannelClient] DataChannel from {} closed", peerId);
    });

    dc->onMessage([this, peerId](auto data) {
        if (std::holds_alternative<std::string>(data)) {
            Log::info("[DataChannelClient] Message from {}: {}", peerId, std::get<std::string>(data));
        } else {
            Log::info("[DataChannelClient] Binary message from {}, size={}", peerId, std::get<rtc::binary>(data).size());
        }
    });

    auto it = m_clients.find(peerId);
    if (it != m_clients.end()) {
        it->second->dataChannel = dc; // 直接设置已有 Client 的 dataChannel
    } else {
        // 理论上不应该走到这里，因为 createPeerConnection 已经创建了
        auto client = std::make_shared<Client>(pc);
        client->dataChannel = dc;
        m_clients.emplace(peerId, client);
    }
}

void DataChannelClient::sendMessage(const std::string& peerId, const std::string& message) {
    auto it = m_clients.find(peerId);
    if (it == m_clients.end()) {
        Log::warn("[DataChannelClient] No DataChannel for peer {}", peerId);
        return;
    }

    auto& client = it->second;
    if (client->dataChannel.has_value() && client->dataChannel.value()->isOpen()) {
        auto dc = client->dataChannel.value();
        if (dc->isOpen()) {
            dc->send(message);
            Log::info("[DataChannelClient] Sent to {}: {}", peerId, message);
        } else {
            Log::warn("[DataChannelClient] DataChannel for {} is not open", peerId);
        }
    } else {
        Log::warn("[DataChannelClient] DataChannel for {} is not open", peerId);
    }
}

void DataChannelClient::close() {
    Log::info("[DataChannelClient] Cleaning up...");
    for (auto& [id, client] : m_clients) {
        if (client->dataChannel.has_value() && client->dataChannel.value()) {
            client->dataChannel.value()->close();
        }
        if (client->peerConnection) {
            client->peerConnection->close();
        }
    }
    m_clients.clear();
    m_connectedFlag = false;
    if (m_ws) {
        m_ws->close();
        m_ws.reset();
    }
}

std::string DataChannelClient::getLocalId() const {
    return m_localId;
}

void DataChannelClient::setOnConnectedCallback(OnConnectedCallback callback) {
    m_onConnectedCallback = std::move(callback);
}


bool DataChannelClient::waitForConnection(std::chrono::milliseconds timeout) {
    if (m_connectedFlag) {
        Log::info("[DataChannelClient] Already connected!");
        return true;
    }
    
    if (!m_connectionPromisePtr) {
        Log::error("[DataChannelClient] No promise set!");
        return false;
    }
    
    auto future = m_connectionPromisePtr->get_future();
    return future.wait_for(timeout) == std::future_status::ready;
}

std::shared_ptr<rtc::PeerConnection> DataChannelClient::createPeerConnection(
    std::weak_ptr<rtc::WebSocket> wws, 
    const std::string& id
) {
    auto pc = std::make_shared<rtc::PeerConnection>(m_config);
    Log::info("[PeerConnection] Created PC for peer: {}", id);


    pc->onStateChange([](rtc::PeerConnection::State state) {
        Log::info("[PeerConnection] State: {}", peerConnectionStateToString(state));
    });

    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        Log::info("[PeerConnection] Gathering State: {}", gatheringStateToString(state));
    });

    pc->onLocalDescription([wws, id](rtc::Description description) {
        Log::info("[PeerConnection] [{}] Generated local {}: {} bytes", 
            id, description.typeString(), std::string(description).size());
        json message = {
            {"id", id},
            {"type", description.typeString()},
            {"description", std::string(description)}
        };
        if (auto ws = wws.lock()) {
            ws->send(message.dump());
            Log::info("[PeerConnection] [{}] Sent local description to signaling server", message.dump()); // 只打印前100字符
        }
    });

    pc->onLocalCandidate([wws, id](rtc::Candidate candidate) {
        Log::debug("[PeerConnection] [{}] ICE Candidate: {} (mid={})", 
                   id, std::string(candidate), candidate.mid());
        json message = {
            {"id", id},
            {"type", "candidate"},
            {"candidate", std::string(candidate)},
            {"mid", candidate.mid()}
        };
        if (auto ws = wws.lock()) ws->send(message.dump());
    });

    pc->onTrack([this, id](std::shared_ptr<rtc::Track> track) {
        Log::info("[PeerConnection] [{}] Received track: {}", 
            id, track->mid());        
                  
        // track->receive([this, id](rtc::binary data, uint32_t timestamp) {
        //     Log::info("[DataChannelClient] Received video frame from {}, size: {}, timestamp: {}", id, data.size(), timestamp);
        //     // 在此添加解码和显示逻辑（如FFmpeg/Qt渲染）
        // });
    });

    pc->onDataChannel([this, id](std::shared_ptr<rtc::DataChannel> dc) {
        Log::info("[PeerConnection] DataChannel from {} received: {}", id, dc->label());


        dc->onOpen([this, id, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
            Log::info("[PeerConnection] DataChannel from {} open", id);
            m_connectedFlag = true;
            if (m_onConnectedCallback) m_onConnectedCallback(id);
            if (auto dc = wdc.lock()) dc->send("Hello from " + m_localId);
        });

        dc->onClosed([this, id]() {
            Log::info("[PeerConnection] DataChannel from {} closed", id);
        });

        dc->onMessage([this, id](auto data) {
            if (std::holds_alternative<std::string>(data)) {
                Log::info("[PeerConnection] Message from {}: {}", id, std::get<std::string>(data));
            } else {
                Log::info("[PeerConnection] Binary message from {}, size={}", id, std::get<rtc::binary>(data).size());
            }
        });

        auto it = m_clients.find(id);
        if (it != m_clients.end()) {
            it->second->dataChannel = dc;
        }
    });

    auto it = m_clients.find(id);
    if (it == m_clients.end()) {
        auto client = std::make_shared<Client>(pc);
        m_clients.emplace(id, client);
    }
    return pc;
}

std::string DataChannelClient::randomId(size_t length) {
    using std::chrono::high_resolution_clock;
    static thread_local std::mt19937 rng(
        static_cast<unsigned int>(high_resolution_clock::now().time_since_epoch().count())
    );
    static const std::string chars("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    std::string id(length, '0');
    std::uniform_int_distribution<int> dist(0, int(chars.size() - 1));
    std::generate(id.begin(), id.end(), [&]() { return chars.at(dist(rng)); });
    return id;
}

std::string DataChannelClient::peerConnectionStateToString(rtc::PeerConnection::State state) {
    switch (state) {
        case rtc::PeerConnection::State::New: return "New";
        case rtc::PeerConnection::State::Connecting: return "Connecting";
        case rtc::PeerConnection::State::Connected: return "Connected";
        case rtc::PeerConnection::State::Disconnected: return "Disconnected";
        case rtc::PeerConnection::State::Failed: return "Failed";
        case rtc::PeerConnection::State::Closed: return "Closed";
        default: return "Unknown";
    }
}

std::string DataChannelClient::gatheringStateToString(rtc::PeerConnection::GatheringState state) {
    switch (state) {
        case rtc::PeerConnection::GatheringState::New: return "New";
        case rtc::PeerConnection::GatheringState::InProgress: return "InProgress";
        case rtc::PeerConnection::GatheringState::Complete: return "Complete";
        default: return "Unknown";
    }
}

void DataChannelClient::myCppLogCallback(rtc::LogLevel level, std::string message) {
    switch (level) {
        case rtc::LogLevel::Fatal:
        case rtc::LogLevel::Error: Log::error("[RTC] {}", message); break;
        case rtc::LogLevel::Warning: Log::warn("[RTC] {}", message); break;
        case rtc::LogLevel::Info: Log::info("[RTC] {}", message); break;
        case rtc::LogLevel::Debug: Log::debug("[RTC] {}", message); break;
        case rtc::LogLevel::Verbose: Log::trace("[RTC] {}", message); break;
        default: Log::info("[RTC] {}", message); break;
    }
}


bool DataChannelClient::hasActiveConnection() const {
    for (const auto& [id, client] : m_clients) {
        if (client->dataChannel.has_value() && 
            client->dataChannel.value() && 
            client->dataChannel.value()->isOpen()) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> DataChannelClient::getConnectedPeers() const {
    std::vector<std::string> connected;
    for (const auto& [id, client] : m_clients) {
        if (client->dataChannel.has_value() && 
            client->dataChannel.value() && 
            client->dataChannel.value()->isOpen()) {
            connected.push_back(id);
        }
    }
    return connected;
}

bool DataChannelClient::isDataChannelOpen(const std::string& peerId) const {
    auto it = m_clients.find(peerId);
    if (it == m_clients.end()) return false;
    
    const auto& client = it->second;
    return client->dataChannel.has_value() && 
           client->dataChannel.value() && 
           client->dataChannel.value()->isOpen();
}