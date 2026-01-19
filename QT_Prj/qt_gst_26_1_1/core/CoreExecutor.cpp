//CoreExecutor.cpp
#include "CoreExecutor.h"
#include "ClientEventFactory.h"
#include <thread>
#include <iostream>

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
