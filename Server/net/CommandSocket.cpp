//CommandSocket.cpp
#include "./net/CommandSocket.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

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
            configureClientSocket(clientfd);
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


void CommandSocket::configureClientSocket(int clientfd) {
    // 1. 设置接收超时
    struct timeval timeout{5, 0}; // 5秒
    setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // 2. 启用 TCP KeepAlive
    int opt = 1;
    setsockopt(clientfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    // 3. KeepAlive 参数
    int keepidle = 10;   // 10 秒没数据开始探测
    int keepintvl = 3;   // 每 3 秒探测一次
    int keepcnt = 3;     // 探测失败 3 次就判定连接死
    setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl));
    setsockopt(clientfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt));
}
