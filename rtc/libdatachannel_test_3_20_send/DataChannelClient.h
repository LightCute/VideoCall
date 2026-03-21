#pragma once

#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <future>
#include <optional>
#include <functional>
#include <thread>
#include <atomic>
#include "utilities/log.h"
#include "h264fileparser.hpp"
#include "helpers.hpp"
class DataChannelClient {
public:
    using OnConnectedCallback = std::function<void(const std::string& peerId)>;

    DataChannelClient();
    ~DataChannelClient();

    DataChannelClient(const DataChannelClient&) = delete;
    DataChannelClient& operator=(const DataChannelClient&) = delete;
    DataChannelClient(DataChannelClient&&) = delete;
    DataChannelClient& operator=(DataChannelClient&&) = delete;

    void connectToServer(const std::string& serverUrl);
    void callPeer(const std::string& peerId);
    void sendMessage(const std::string& peerId, const std::string& message);
    void close();
    std::string getLocalId() const;

    bool waitForConnection(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

    bool hasActiveConnection() const;
    std::vector<std::string> getConnectedPeers() const;
    bool isDataChannelOpen(const std::string& peerId) const;

private:
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(
        std::weak_ptr<rtc::WebSocket> wws, 
        const std::string& id
    );
    static std::string randomId(size_t length);
    static std::string peerConnectionStateToString(rtc::PeerConnection::State state);
    static std::string gatheringStateToString(rtc::PeerConnection::GatheringState state);
    static void myCppLogCallback(rtc::LogLevel level, std::string message);

    void addToStream(std::shared_ptr<Client> client, bool isAddingVideo);
    void startStream();
    template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }
    void sendInitialNalus(std::shared_ptr<Stream> stream, std::shared_ptr<ClientTrackData> video);
    std::shared_ptr<ClientTrackData> addVideo(const std::shared_ptr<rtc::PeerConnection> pc, 
        const uint8_t payloadType, 
        const uint32_t ssrc, 
        const std::string cname, 
        const std::string msid, 
        const std::function<void (void)> onOpen);

    std::shared_ptr<ClientTrackData> addAudio(const std::shared_ptr<rtc::PeerConnection> pc, 
        const uint8_t payloadType, 
        const uint32_t ssrc, 
        const std::string cname, 
        const std::string msid, 
        const std::function<void (void)> onOpen);

    uint32_t generateUniqueSSRC(const std::string& clientId);
    std::shared_ptr<Stream> createStream(
    const std::string h264Samples, 
    const unsigned fps, 
    const std::string opusSamples);


    std::shared_ptr<std::promise<void>> m_connectionPromisePtr;
    std::atomic<bool> m_connectedFlag{false};

    std::string m_localId;
    rtc::Configuration m_config;
    std::shared_ptr<rtc::WebSocket> m_ws;
    std::promise<void> m_wsPromise;

    // 视频传输成员变量
    std::shared_ptr<rtc::Track> videoTrack;
    std::shared_ptr<rtc::RtpPacketizationConfig> rtpConfig;
    std::shared_ptr<rtc::H264RtpPacketizer> packetizer;
    std::shared_ptr<H264FileParser> h264Parser;
    std::thread videoSendThread;
    std::atomic<bool> sendingVideo{false};

    std::unordered_map<std::string, std::shared_ptr<Client>> m_clients;
    std::optional<std::shared_ptr<Stream>> m_avStream;
    DispatchQueue m_MainThread;
};