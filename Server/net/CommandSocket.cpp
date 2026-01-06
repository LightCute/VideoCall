//CommandSocket.cpp
#include "./net/CommandSocket.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

bool CommandSocket::startListen(int port) {
    listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd_ < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listenfd_, (sockaddr*)&addr, sizeof(addr)) < 0) return false;
    if (listen(listenfd_, 16) < 0) return false;

    running_ = true;
    acceptThread_ = std::thread(&CommandSocket::acceptThreadFunc, this);
    return true;
}

void CommandSocket::setAcceptCallback(AcceptCallback cb) {
    acceptCb_ = std::move(cb);
}

void CommandSocket::acceptThreadFunc() {
    while (running_) {
        int clientfd = accept(listenfd_, nullptr, nullptr);
        if (clientfd >= 0 && acceptCb_) {
            acceptCb_(clientfd);   // ⭐ 抛给 LoginServer
        }
    }
}

void CommandSocket::stop() {
    running_ = false;
    if (acceptThread_.joinable())
        acceptThread_.join();
    if (listenfd_ >= 0)
        close(listenfd_);
}

void CommandSocket::sendPacket(int fd, const std::string& payload) {
    auto data = PacketCodec::encode(payload);
    send(fd, data.data(), data.size(), 0);
}


void CommandSocket::sendMessage(const std::string& msg) {
    sendPacket(listenfd_,msg);
}


