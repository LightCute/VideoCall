#pragma once
#include <memory>

#include <rtc/rtc.hpp>

class WebSocket {
public:
    WebSocket();

    void connect2Server (const std::string& url, std::string& localId) ;
    void send(const std::string& msg) ;
    void close() ;



    void connect2Peer(const std::string& peerId) ;
    void send2Peer(const std::string& peerId, const std::string& msg) ;
private:
    std::shared_ptr<rtc::WebSocket> m_ws;
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(const rtc::Configuration &config,
                                                                    std::weak_ptr<rtc::WebSocket> wws, std::string id);
    std::string m_localId;
    template <class T> std::weak_ptr<T> make_weak_ptr(std::shared_ptr<T> ptr) { return ptr; }
    std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> m_peerConnectionMap;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> m_dataChannelMap;
    std::string m_peer_id;
    rtc::Configuration m_config;

};
