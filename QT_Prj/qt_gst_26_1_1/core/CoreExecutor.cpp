//CoreExecutor.cpp
#include "CoreExecutor.h"
#include "ClientEventFactory.h"
#include <thread>
#include <iostream>
#include "./util/GetLocalIP.h"

CoreExecutor::CoreExecutor(InputCallback cb)
    : postInput_(std::move(cb)) {
    initSocketCallbacks();
    heartbeatThread_ = std::thread(&CoreExecutor::heartbeatLoop, this);
}

CoreExecutor::~CoreExecutor() {
    stop();
}

void CoreExecutor::initSocketCallbacks() {
    // 1. 连接成功回调：推送 InTcpConnected 到 Core
    socket_.setConnectCallback([this]() {
        if (isRunning_) {
            postInput_(core::InTcpConnected{}); // 替换为 InTcpConnected
            std::cout << "[Executor] Socket connected, push InTcpConnected to Core" << std::endl;
        }
    });

    // 2. 断开连接回调：推送 InTcpDisconnected 到 Core
    socket_.setDisconnectCallback([this]() {
        if (isRunning_) {
            postInput_(core::InTcpDisconnected{}); // 替换为 InTcpDisconnected
            std::cout << "[Executor] Socket disconnected, push InTcpDisconnected to Core" << std::endl;
        }
    });

    // 3. 消息接收回调：解析消息并推送对应 CoreInput 到 Core
    socket_.setMessageCallback([this](const std::string& msg) {
        if (!isRunning_) return;

        ClientEvent event = ClientEventFactory::makeEvent(msg);
        std::visit([this](auto&& e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, ProtoEvtLoginOk>) {
                postInput_(core::InLoginOk{});
            } else if constexpr (std::is_same_v<T, ProtoEvtLoginFail>) {
                postInput_(core::InLoginFail{e.resp.message});
            } else if constexpr (std::is_same_v<T, ProtoEvtOnlineUsers>) {
                core::InOnlineUsers in;

                for (const auto& u : (e.users.users)) {
                    in.users.push_back(core::OnlineUser{
                        .name = u.username,
                        .privilege = u.privilege
                    });
                }

                postInput_(std::move(in));
            }
            else if constexpr (std::is_same_v<T, ProtoEvHeartbeatAck>) {
                postInput_(core::InHeartbeatOk{});
            }
            else if constexpr (std::is_same_v<T, ProtoEvtForwardText>) {
                postInput_(core::InForwardText{e.from_user, e.content});
            }

            else if constexpr (std::is_same_v<T, ProtoEvtCallIncoming>)
                postInput_(core::InCallIncoming{e.from});
            else if constexpr (std::is_same_v<T, ProtoEvtCallAccepted>)
                postInput_(core::InCallAccepted{e.peer});
            else if constexpr (std::is_same_v<T, ProtoEvtCallRejected>)
                postInput_(core::InCallRejected{e.peer});
            else if constexpr (std::is_same_v<T, ProtoEvtMediaPeer>)
                postInput_(core::InMediaPeer{e.peer, e.lanIp, e.vpnIp, e.udpPort});
            else {
                postInput_(core::InUnknow{});
            }

        }, event);
    });
}

void CoreExecutor::connectToServer(const std::string& host, int port) {
    // 异步执行连接
    std::thread([this, host, port]() {
        if (!socket_.connectToServer(host, port)) {
            // 连接失败：推送 InTcpDisconnected
            postInput_(core::InTcpDisconnected{}); // 替换为 InTcpDisconnected
            std::cout << "[Executor] Connect to " << host << ":" << port << " failed" << std::endl;
        }
        else {
            std::cout << "[Executor] Successfully connected to server: " << host << ":" << port << std::endl;
        }
    }).detach();
}

void CoreExecutor::sendLoginRequest(const std::string& user, const std::string& pass) {
    std::string loginMsg = proto::makeLoginRequest(user, pass);
    socket_.sendMessage(loginMsg);
    std::cout << "[Executor] Send login request for user: " << user << std::endl;
}

void CoreExecutor::heartbeatLoop() {
    while (isRunning_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        if (!isRunning_) break;

        // 推送 HeartbeatTick 给 Core
        postInput_(core::InHeartbeatTick{});
        std::cout << "[Executor] push InHeartbeatTick" << std::endl;
    }
}


void CoreExecutor::sendPing() {
    std::string ping = proto::makeHeartbeat();
    socket_.sendMessage(ping);
    std::cout << "[Executor] Send PING" << std::endl;
}


void CoreExecutor::stop() {
    isRunning_ = false;
    socket_.stop();
}

void CoreExecutor::setLanMode() {
    mode_ = NetMode::LAN;
    std::cout << "[Executor] Switched to LAN mode\n";
}

void CoreExecutor::setVpnMode() {
    mode_ = NetMode::VPN;
    std::cout << "[Executor] Switched to VPN mode\n";
}

void CoreExecutor::sendLocalIP() {
    // std::string ping = proto::makeHeartbeat();
    // socket_.sendMessage(ping);
    std::string lanIp = getLocalLanIP();   // 你需要实现
    std::string vpnIp = getVpnIp();        // 先返回 ""

    int port = 5000;

    std::string msg = proto::makeRegisterPeerMsg(lanIp, vpnIp, port);
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send Local IP" << std::endl;
}

void CoreExecutor::sendTextMsg(const std::string& target_user, const std::string& content) {
    std::string textMsg = proto::makeSendTextMsg(target_user, content);
    socket_.sendMessage(textMsg);
    std::cout << "[Executor] Send text msg to " << target_user << ": " << content << std::endl;
}

// 发送CALL请求
void CoreExecutor::sendCallRequest(const std::string& target_user) {
    std::string msg = proto::makeCallRequest(target_user);
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send CALL request to: " << target_user << std::endl;
}

// 发送CALL_ACCEPT
void CoreExecutor::sendAcceptCall() {
    std::string msg = proto::makeAcceptCallRequest();
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send CALL_ACCEPT" << std::endl;
}

// 发送CALL_REJECT
void CoreExecutor::sendRejectCall() {
    std::string msg = proto::makeRejectCallRequest();
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send CALL_REJECT" << std::endl;
}

// 发送MEDIA_OFFER
void CoreExecutor::sendMediaOffer(const std::string& peer) {
    std::string msg = proto::makeMediaOfferRequest(peer);
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send MEDIA_OFFER to: " << peer << std::endl;
}

// 发送MEDIA_ANSWER
void CoreExecutor::sendMediaAnswer(const std::string& peer) {
    std::string msg = proto::makeMediaAnswerRequest(peer);
    socket_.sendMessage(msg);
    std::cout << "[Executor] Send MEDIA_ANSWER to: " << peer << std::endl;
}
