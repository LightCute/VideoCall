#include "LoginServer.h"
#include "./protocol/protocol_text.h"
#include "./protocol/protocol_common.h"
using namespace proto;

bool LoginServer::start(int port) {
    listener_.setAcceptCallback(
        [this](int clientfd) {
            onAccept(clientfd);
        }
    );
    return listener_.startListen(port);
}

void LoginServer::sendPacket(int fd, const std::string& payload) {
    auto data = PacketCodec::encode(payload);
    send(fd, data.data(), data.size(), 0);
}


void LoginServer::onAccept(int clientfd) {
    std::cout << "[Server] new client fd=" << clientfd << std::endl;

    pool_.post([this, clientfd] {
        clientThread(clientfd);
    });
}

void LoginServer::clientThread(int clientfd) {
    std::vector<char> recvBuffer;
    char buf[1024];

    while (true) {
        int n = recv(clientfd, buf, sizeof(buf), 0);
        if (n <= 0) break;

        recvBuffer.insert(recvBuffer.end(), buf, buf + n);

        std::string payload;
        while (PacketCodec::tryDecode(recvBuffer, payload)) {
            onMessage(clientfd, payload);
        }
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        clients_.erase(clientfd);
    }


    //broadcastOnlineUsers();
    close(clientfd);
    std::cout << "[Server] client disconnected fd=" << clientfd << std::endl;
}

void LoginServer::onMessage(int clientfd, const std::string& msg) {
    std::cout << "[Server] recv from " << clientfd << ": " << msg << std::endl;

    std::string user, pwd;
    if (proto::parseLoginRequest(msg, user, pwd) == true) {
        proto::UserInfo user_temp;

        if(user == "admin" && pwd == "123")
        {
            std::cout << "Login suceess admin 123" << std::endl;
            user_temp.username = "admin";
            user_temp.privilege = 10;

            {
                std::lock_guard<std::mutex> lock(mutex_);
                clients_[clientfd] = {
                    user,
                    user_temp.privilege,
                    "unknown",
                    0,
                    true
                };
            }

            sendPacket(clientfd, proto::makeLoginOk(user_temp, "welcome"));

            broadcastOnlineUsers();
        }
        else {
            sendPacket(clientfd, proto::makeLoginFail("Login fail !!"));
        }


    }    

    
}


void LoginServer::broadcastOnlineUsers()
{
    std::string msg = "ONLINE_USERS ";
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // 统计在线用户
        int onlineCount = 0;
        for (auto &[fd, info] : clients_) {
            if (info.online) onlineCount++;
        }
        msg += std::to_string(onlineCount) + " ";

        // 拼用户名列表
        for (auto &[fd, info] : clients_) {
            if (info.online) {
                msg += info.username + " ";
            }
        }
    }
    msg += "\n";

    // 发送给所有在线客户端
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &[fd, info] : clients_) {
        if (info.online) {
            sendPacket(fd, msg);
        }
    }
}

