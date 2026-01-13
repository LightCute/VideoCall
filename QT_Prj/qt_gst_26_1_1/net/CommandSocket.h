//CommandSocket.h
#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>
#include "PacketCodec.h"

class CommandSocket {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using ConnectCallback = std::function<void()>;
    using DisconnectCallback = std::function<void()>;

    CommandSocket();
    ~CommandSocket();

    void setConnectCallback(ConnectCallback cb);
    void setDisconnectCallback(DisconnectCallback cb);
    // TCP 客户端
    bool connectToServer(const std::string& host, int port);

    // TCP 服务器
    bool startServer(int port);

    void setMessageCallback(MessageCallback cb);

    // 发送数据
    void sendMessage(const std::string& msg);

    // 停止
    void stop();

private:
    void clientThreadFunc();
    void serverThreadFunc();
    void sendPacket(int fd, const std::string& payload);
    int sockfd_ = -1;
    int clientfd_ = -1;  // 对于服务器端保存客户端连接
    std::thread workerThread_;
    std::atomic<bool> running_{false};
    MessageCallback callback_;

    ConnectCallback onConnect_;
    DisconnectCallback onDisconnect_;

};
