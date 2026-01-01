#include "CommandSocket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

CommandSocket::CommandSocket() {}

CommandSocket::~CommandSocket() {
    stop();
}

void CommandSocket::setMessageCallback(MessageCallback cb) {
    callback_ = std::move(cb);
}

bool CommandSocket::connectToServer(const std::string& host, int port) {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) return false;

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);

    if (connect(sockfd_, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd_);
        sockfd_ = -1;
        return false;
    }

    running_ = true;
    workerThread_ = std::thread(&CommandSocket::clientThreadFunc, this);
    return true;
}

bool CommandSocket::startServer(int port) {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd_, (sockaddr*)&addr, sizeof(addr)) < 0) return false;
    if (listen(sockfd_, 1) < 0) return false;

    running_ = true;
    workerThread_ = std::thread(&CommandSocket::serverThreadFunc, this);
    return true;
}

bool CommandSocket::sendMessage(const std::string& msg) {
    if (sockfd_ < 0 && clientfd_ < 0) return false;

    int fd = (clientfd_ >= 0) ? clientfd_ : sockfd_;
    int n = send(fd, msg.c_str(), msg.size(), 0);
    return n == (int)msg.size();
}

void CommandSocket::stop() {
    running_ = false;
    if (workerThread_.joinable())
        workerThread_.join();
    if (sockfd_ >= 0) close(sockfd_);
    if (clientfd_ >= 0) close(clientfd_);
    sockfd_ = -1;
    clientfd_ = -1;
}

void CommandSocket::clientThreadFunc() {
    char buffer[1024];
    while (running_) {
        int n = recv(sockfd_, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            if (callback_) callback_(std::string(buffer));
        } else if (n == 0) {
            break; // 服务器断开
        }
    }
}

void CommandSocket::serverThreadFunc() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    clientfd_ = accept(sockfd_, (sockaddr*)&client_addr, &client_len);
    if (clientfd_ < 0) return;

    char buffer[1024];
    while (running_) {
        int n = recv(clientfd_, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            if (callback_) callback_(std::string(buffer));
        } else if (n == 0) {
            break; // 客户端断开
        }
    }
}
