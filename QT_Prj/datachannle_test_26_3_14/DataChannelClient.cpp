#include "DataChannelClient.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <thread>

using namespace std::chrono_literals;
using nlohmann::json;

DataChannelClient::DataChannelClient() {
    // 初始化日志
    Log::init("app.log", Log::Mode::Async, spdlog::level::trace);
    Log::info("[DataChannelClient] Starting, thread id: [{}]", Log::threadIdToString(std::this_thread::get_id()));

    // 初始化 RTC 日志
    rtc::InitLogger(rtc::LogLevel::Debug, myCppLogCallback);

    // 生成本地 ID
    m_localId = randomId(4);
    Log::info("[DataChannelClient] Local ID generated: {}", m_localId);

    // 配置 STUN 服务器（可根据需要修改）
    m_config.iceServers.emplace_back("stun:stun.l.google.com:19302");
}

DataChannelClient::~DataChannelClient() {
    close();
}

void DataChannelClient::connectToServer(const std::string& serverUrl) {
    if (m_ws && m_ws->isOpen()) {
        Log::warn("[DataChannelClient] WebSocket is already connected");
        return;
    }

    // 创建 WebSocket
    m_ws = std::make_shared<rtc::WebSocket>();

    // 重置 Promise 用于等待连接
    m_wsPromise = std::promise<void>();
    auto wsFuture = m_wsPromise.get_future();

    // 设置 WebSocket 回调
    m_ws->onOpen([this]() {
        Log::info("[DataChannelClient] WebSocket connected, signaling ready");
        m_wsPromise.set_value();
    });

    m_ws->onError([this](std::string s) {
        Log::error("[DataChannelClient] WebSocket error: {}", s);
        m_wsPromise.set_exception(std::make_exception_ptr(std::runtime_error(s)));
    });

    m_ws->onClosed([this]() {
        Log::info("[DataChannelClient] WebSocket closed");
    });

    m_ws->onMessage([this, wws = std::weak_ptr<rtc::WebSocket>(m_ws)](auto data) {
        // 处理信令消息
        if (!std::holds_alternative<std::string>(data)) return;

        json message = json::parse(std::get<std::string>(data));
        auto it = message.find("id");
        if (it == message.end()) return;
        auto id = it->get<std::string>();

        it = message.find("type");
        if (it == message.end()) return;
        auto type = it->get<std::string>();

        // 获取或创建 PeerConnection
        std::shared_ptr<rtc::PeerConnection> pc;
        if (auto jt = m_peerConnectionMap.find(id); jt != m_peerConnectionMap.end()) {
            pc = jt->second;
        } else if (type == "offer") {
            Log::info("[DataChannelClient] Answering to {}", id);
            pc = createPeerConnection(wws, id);
        } else {
            return;
        }

        // 处理 SDP 或 Candidate
        if (type == "offer" || type == "answer") {
            auto sdp = message["description"].get<std::string>();
            pc->setRemoteDescription(rtc::Description(sdp, type));
        } else if (type == "candidate") {
            auto sdp = message["candidate"].get<std::string>();
            auto mid = message["mid"].get<std::string>();
            pc->addRemoteCandidate(rtc::Candidate(sdp, mid));
        }
    });

    // 连接服务器（URL 格式：ws://ip:port/localId）
    std::string fullUrl = serverUrl + "/" + m_localId;
    m_ws->open(fullUrl);

    Log::info("[DataChannelClient] Waiting for signaling connection...");
    wsFuture.get(); // 阻塞等待连接完成
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

    Log::info("[DataChannelClient] Offering to {}", peerId);
    auto pc = createPeerConnection(m_ws, peerId);

    // 创建 DataChannel
    const std::string label = "test";
    Log::info("[DataChannelClient] Creating DataChannel with label \"{}\"", label);
    auto dc = pc->createDataChannel(label);

    // 设置 DataChannel 回调
    dc->onOpen([this, peerId, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
        Log::info("[DataChannelClient] DataChannel from {} open", peerId);
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

    m_dataChannelMap.emplace(peerId, dc);
}

void DataChannelClient::sendMessage(const std::string& peerId, const std::string& message) {
    auto it = m_dataChannelMap.find(peerId);
    if (it == m_dataChannelMap.end()) {
        Log::warn("[DataChannelClient] No DataChannel for peer {}", peerId);
        return;
    }

    if (auto dc = it->second; dc && dc->isOpen()) {
        dc->send(message);
        Log::info("[DataChannelClient] Sent to {}: {}", peerId, message);
    } else {
        Log::warn("[DataChannelClient] DataChannel for {} is not open", peerId);
    }
}

void DataChannelClient::close() {
    Log::info("[DataChannelClient] Cleaning up...");
    m_dataChannelMap.clear();
    m_peerConnectionMap.clear();
    if (m_ws) {
        m_ws->close();
        m_ws.reset();
    }
}

std::string DataChannelClient::getLocalId() const {
    return m_localId;
}

// ------------------------------ 私有辅助函数 ------------------------------
std::shared_ptr<rtc::PeerConnection> DataChannelClient::createPeerConnection(
    std::weak_ptr<rtc::WebSocket> wws, 
    const std::string& id
) {
    auto pc = std::make_shared<rtc::PeerConnection>(m_config);

    // PeerConnection 状态回调
    pc->onStateChange([](rtc::PeerConnection::State state) {
        Log::info("[DataChannelClient] State: {}", peerConnectionStateToString(state));
    });

    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        Log::info("[DataChannelClient] Gathering State: {}", gatheringStateToString(state));
    });

    // 本地 SDP 回调
    pc->onLocalDescription([wws, id](rtc::Description description) {
        json message = {
            {"id", id},
            {"type", description.typeString()},
            {"description", std::string(description)}
        };
        if (auto ws = wws.lock()) ws->send(message.dump());
    });

    // 本地 Candidate 回调
    pc->onLocalCandidate([wws, id](rtc::Candidate candidate) {
        json message = {
            {"id", id},
            {"type", "candidate"},
            {"candidate", std::string(candidate)},
            {"mid", candidate.mid()}
        };
        if (auto ws = wws.lock()) ws->send(message.dump());
    });

    // 接收 DataChannel 回调
    pc->onDataChannel([this, id](std::shared_ptr<rtc::DataChannel> dc) {
        Log::info("[DataChannelClient] DataChannel from {} received: {}", id, dc->label());

        dc->onOpen([this, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
            if (auto dc = wdc.lock()) dc->send("Hello from " + m_localId);
        });

        dc->onClosed([this, id]() {
            Log::info("[DataChannelClient] DataChannel from {} closed", id);
        });

        dc->onMessage([this, id](auto data) {
            if (std::holds_alternative<std::string>(data)) {
                Log::info("[DataChannelClient] Message from {}: {}", id, std::get<std::string>(data));
            } else {
                Log::info("[DataChannelClient] Binary message from {}, size={}", id, std::get<rtc::binary>(data).size());
            }
        });

        m_dataChannelMap.emplace(id, dc);
    });

    m_peerConnectionMap.emplace(id, pc);
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