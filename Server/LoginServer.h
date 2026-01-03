#pragma once
#include "CommandSocket.h"
#include <thread>
#include <mutex>
#include <map>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "./protocol/LoginProtocol.h"
#include "ThreadPool.h"
#include "./protocol/PacketCodec.h"

class LoginServer {
public:
    bool start(int port);

private:
    void onAccept(int clientfd);
    void clientThread(int clientfd);
    void onMessage(int clientfd, const std::string& msg);
    void broadcastOnlineUsers();
    void sendPacket(int fd, const std::string& payload);
    struct ClientInfo {
        std::string username;
        int privilegeLevel;
        std::string ip;
        int port;
        bool online;
    };

    std::map<int, ClientInfo> clients_;  //fd -> ClientInfo

    CommandSocket listener_;
    std::mutex mutex_;
    ThreadPool pool_{8};
};
