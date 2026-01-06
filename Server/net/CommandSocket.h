//CommandSocket.h
#pragma once
#include <functional>
#include <thread>
#include <atomic>
#include "./net/PacketCodec.h"

class CommandSocket {
public:
    using AcceptCallback = std::function<void(int)>;

    bool startListen(int port);
    void setAcceptCallback(AcceptCallback cb);
    void stop();
    void sendPacket(int fd, const std::string& payload);
    void sendMessage(const std::string& msg);
    
private:
    void acceptThreadFunc();
    int listenfd_ = -1;
    std::thread acceptThread_;
    std::atomic<bool> running_{false};
    AcceptCallback acceptCb_;
};
