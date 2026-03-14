#pragma once
#include <memory>

#include "../service/abstract_net.h"
#include <rtc/rtc.hpp>

class WebSocket : public AbstractNet {
public:
    WebSocket();

    void connect(const std::string& url) override;
    void send(const std::string& msg) override;
    void close() override;

    void onMessage(MessageCallback cb) override;
    void onOpen(OpenCallback cb) override;
    void onClose(CloseCallback cb) override;
    void onError(ErrorCallback cb) override;

    void connect2Peer(const std::string& peerId) override;
    void send2Peer(const std::string& peerId, const std::string& msg) override;
private:
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Configuration &config,
                                                                    std::weak_ptr<rtc::WebSocket> wws, std::string id);
    std::string localId;
    template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }
    std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> peerConnectionMap;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> dataChannelMap;
    std::string m_peer_id;
    std::shared_ptr<rtc::WebSocket> ws;
    rtc::Configuration config;
    MessageCallback messageCallback;
    OpenCallback openCallback;
    CloseCallback closeCallback;
    ErrorCallback errorCallback;

    void initCallbacks();
};
