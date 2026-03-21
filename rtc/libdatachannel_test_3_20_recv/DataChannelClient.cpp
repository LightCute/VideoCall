#include "DataChannelClient.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <stdexcept>
#include <thread>
#include "opusfileparser.hpp"

using namespace std::chrono_literals;
using nlohmann::json;

DataChannelClient::DataChannelClient() 
    : m_MainThread("MainThread")
{
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

    pc->onTrack([this, id, weak_pc = std::weak_ptr<rtc::PeerConnection>(pc)](std::shared_ptr<rtc::Track> track) {
        Log::info("[PeerConnection] [{}] Received track, mid: {}", 
            id, track->mid());

        // 2. 线程安全地保存 Track 到 Client
        auto it_client = m_clients.find(id);
        if (it_client == m_clients.end()) {
            Log::error("[PeerConnection] [{}] Client not found for incoming track", id);
            return;
        }
        auto client = it_client->second;

        bool isVideo = (track->description().type() == "video");

        if (isVideo) {
            client->recvVideo = track;
            Log::info("[PeerConnection] [{}] Saved VIDEO track", id);
            
            // 设置 H.264 解包器
            auto depacketizer = std::make_shared<rtc::H264RtpDepacketizer>();
            track->setMediaHandler(depacketizer);
        } else {
            client->recvAudio = track;
            Log::info("[PeerConnection] [{}] Saved AUDIO track", id);
            
            auto depacketizer = std::make_shared<rtc::OpusRtpDepacketizer>();
            track->setMediaHandler(depacketizer);
            // 如果是 Opus，设置 Opus 解包器 (如果 libdatachannel 提供的话)
            // 注意：如果是 RAW 音频或其他格式，可能不需要 Depacketizer，或者需要对应的 Depacketizer
            // auto depacketizer = std::make_shared<rtc::OpusDepacketizer>(); 
            // track->setMediaHandler(depacketizer);
        }

        bool haveVideo = (client->recvVideo != nullptr);
        bool haveAudio = (client->recvAudio != nullptr);

        if (!client->player) {
            client->player = std::make_shared<GstMediaPlayer>();
            // 先不 start，等数据来？或者现在就 start？建议现在 start，让 Pipeline 跑起来
            client->player->start();
            Log::info("[PeerConnection] [{}] Created GstMediaPlayer", id);
        }
        // 3. 设置 Track 的回调（关键！）
        // 3.1 Track 打开回调
        track->onOpen([this, id, weak_client = std::weak_ptr<Client>(client)]() {
            Log::info("[PeerConnection] [{}] Incoming track opened", id);            
            // (可选) 在这里可以通知 UI 或更新状态
            // if (auto c = weak_client.lock()) {
            //     // 创建播放器
            //     c->player = std::make_shared<GstVideoPlayer>();
            //     if (!c->player->start()) {
            //         Log::error("[PeerConnection] [{}] Failed to start GStreamer player", id);
            //     }
            // }
        });

        // 3.2 Track 关闭回调
        track->onClosed([this, id, weak_client = std::weak_ptr<Client>(client)]() {
            Log::info("[PeerConnection] [{}] Incoming track closed", id);
            // (可选) 在这里可以通知 UI 或更新状态
            if (auto c = weak_client.lock()) {
                if (c->player) {
                    c->player->stop();
                }
            }
        });

        // 3.3 ✅ 核心：媒体数据接收回调（在这里处理解码/渲染）
        track->onFrame([this, id, client, isVideo](rtc::binary data, rtc::FrameInfo info) {
            if (!client->player) return;

            if (data.empty()) {
                Log::warn("[PeerConnection] [{}] {} frame is empty!", id, (isVideo ? "VIDEO" : "AUDIO"));
                return;
            }

            if (isVideo) {
                client->player->pushVideoFrame(std::move(data), info.timestamp);
                // 视频日志可以少一点
                static int v_cnt = 0;
                if(++v_cnt % 30 == 0) Log::debug("[PeerConnection] [{}] Video OK, size={}", id, data.size());
            } else {
                // ✅✅✅ 音频：每次都打印，确保这里在执行！
                Log::warn("[PeerConnection] [{}] 🔊 Pushing AUDIO, size={}, timestamp={}", id, data.size(), info.timestamp);
                client->player->pushAudioFrame(std::move(data), info.timestamp);
            }
        });


    });

    pc->onDataChannel([this, id](std::shared_ptr<rtc::DataChannel> dc) {
        Log::info("[PeerConnection] DataChannel from {} received: {}", id, dc->label());


        dc->onOpen([this, id, wdc = std::weak_ptr<rtc::DataChannel>(dc)]() {
            Log::info("[PeerConnection] DataChannel from {} open", id);
            m_connectedFlag = true;
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







//---------------------------------------------------------


std::shared_ptr<ClientTrackData> DataChannelClient::addVideo(const std::shared_ptr<rtc::PeerConnection> pc, 
    const uint8_t payloadType, 
    const uint32_t ssrc, 
    const std::string cname, 
    const std::string msid, 
    const std::function<void (void)> onOpen) {
    auto video = rtc::Description::Video(cname, rtc::Description::Direction::SendRecv);
    video.addH264Codec(payloadType);
    video.addSSRC(ssrc, cname, msid, cname);
    auto track = pc->addTrack(video);
    // create RTP configuration
    auto rtpConfig = std::make_shared<rtc::RtpPacketizationConfig>(ssrc, cname, payloadType, rtc::H264RtpPacketizer::ClockRate);
    // create packetizer
    auto packetizer = std::make_shared<rtc::H264RtpPacketizer>(rtc::NalUnit::Separator::Length, rtpConfig);
    // add RTCP SR handler
    auto srReporter = std::make_shared<rtc::RtcpSrReporter>(rtpConfig);
    packetizer->addToChain(srReporter);
    // add RTCP NACK handler
    auto nackResponder = std::make_shared<rtc::RtcpNackResponder>();
    packetizer->addToChain(nackResponder);
    // set handler
    track->setMediaHandler(packetizer);
    track->onOpen(onOpen);
    auto trackData = std::make_shared<ClientTrackData>(track, srReporter);
    return trackData;
}






//----------------------------------------------------



/// Start stream
void DataChannelClient::startStream() {
    std::shared_ptr<Stream> stream;
    if (m_avStream.has_value()) {
        stream = m_avStream.value();
        if (stream->isRunning) {
            // stream is already running
            return;
        }
    } else {
        stream = createStream("samples/h264", 30, "samples/opus");
        m_avStream = stream;
    }
    stream->start();
}

/// Add client to stream
/// @param client Client
/// @param adding_video True if adding video
void DataChannelClient::addToStream(std::shared_ptr<Client> client, bool isAddingVideo) {
    if (client->getState() == Client::State::Waiting) {
        client->setState(isAddingVideo ? Client::State::WaitingForAudio : Client::State::WaitingForVideo);
    } else if ((client->getState() == Client::State::WaitingForAudio && !isAddingVideo)
               || (client->getState() == Client::State::WaitingForVideo && isAddingVideo)) {

        // Audio and video tracks are collected now
        assert(client->video.has_value() && client->audio.has_value());
        auto video = client->video.value();

        if (m_avStream.has_value()) {
            sendInitialNalus(m_avStream.value(), video);
        }

        client->setState(Client::State::Ready);
    }
    if (client->getState() == Client::State::Ready) {
        startStream();
    }
}




/// Create stream
std::shared_ptr<Stream> DataChannelClient::createStream(
    const std::string h264Samples, 
    const unsigned fps, 
    const std::string opusSamples) {
    // video source
    auto video = std::make_shared<H264FileParser>(h264Samples, fps, true);
    // audio source
    auto audio = std::make_shared<OPUSFileParser>(opusSamples, true);

    auto stream = std::make_shared<Stream>(video, audio);
    // set callback responsible for sample sending
    stream->onSample([this, ws = make_weak_ptr(stream)](Stream::StreamSourceType type, uint64_t sampleTime, rtc::binary sample) {
        std::vector<ClientTrack> tracks{};
        std::string streamType = type == Stream::StreamSourceType::Video ? "video" : "audio";
        // get track for given type
        std::function<std::optional<std::shared_ptr<ClientTrackData>> (std::shared_ptr<Client>)> getTrackData = [type](std::shared_ptr<Client> client) {
            return type == Stream::StreamSourceType::Video ? client->video : client->audio;
        };
        // get all clients with Ready state
        for(auto id_client: m_clients) {
            auto id = id_client.first;
            auto client = id_client.second;
            auto optTrackData = getTrackData(client);
            if (client->getState() == Client::State::Ready && optTrackData.has_value()) {
                auto trackData = optTrackData.value();
                tracks.push_back(ClientTrack(id, trackData));
            }
        }
        if (!tracks.empty()) {
            for (auto clientTrack: tracks) {
                auto client = clientTrack.id;
                auto trackData = clientTrack.trackData;

                std::cout << "Sending " << streamType << " sample with size: " << std::to_string(sample.size()) << " to " << client << std::endl;
                try {
                    // send sample
                    trackData->track->sendFrame(sample, std::chrono::duration<double, std::micro>(sampleTime));
                } catch (const std::exception &e) {
                    std::cerr << "Unable to send "<< streamType << " packet: " << e.what() << std::endl;
                }
            }
        }
        m_MainThread.dispatch([this,ws]() {
            if (m_clients.empty()) {
                // we have no clients, stop the stream
                if (auto stream = ws.lock()) {
                    stream->stop();
                }
            }
        });
    });
    return stream;
}


/// Send previous key frame so browser can show something to user
/// @param stream Stream
/// @param video Video track data
void DataChannelClient::sendInitialNalus(std::shared_ptr<Stream> stream, std::shared_ptr<ClientTrackData> video) {
    auto h264 = dynamic_cast<H264FileParser *>(stream->video.get());
    auto initialNalus = h264->initialNALUS();

    // send previous NALU key frame so users don't have to wait to see stream works
    if (!initialNalus.empty()) {
        const double frameDuration_s = double(h264->getSampleDuration_us()) / (1000 * 1000);
        const uint32_t frameTimestampDuration = video->sender->rtpConfig->secondsToTimestamp(frameDuration_s);
        video->sender->rtpConfig->timestamp = video->sender->rtpConfig->startTimestamp - frameTimestampDuration * 2;
        video->track->send(initialNalus);
        video->sender->rtpConfig->timestamp += frameTimestampDuration;
        // Send initial NAL units again to start stream in firefox browser
        video->track->send(initialNalus);
    }
}


uint32_t DataChannelClient::generateUniqueSSRC(const std::string& clientId) {
    // 方法 1: 基于客户端 ID 哈希
    std::hash<std::string> hasher;
    return hasher(clientId) & 0xFFFFFFFF;
    
    // 方法 2: 随机生成
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<uint32_t> dist(1, 0xFFFFFFFE);
    // return dist(gen);
}

