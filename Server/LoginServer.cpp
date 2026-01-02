#include "LoginServer.h"


bool LoginServer::start(int port) {
    listener_.setAcceptCallback(
        [this](int clientfd) {
            onAccept(clientfd);
        }
    );
    return listener_.startListen(port);
}

void LoginServer::onAccept(int clientfd) {
    std::cout << "[Server] new client fd=" << clientfd << std::endl;
    std::thread(&LoginServer::clientThread, this, clientfd).detach();
}

void LoginServer::clientThread(int clientfd) {
    char buf[1024];

    while (true) {
        int n = recv(clientfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;

        buf[n] = 0;
        onMessage(clientfd, std::string(buf));
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        clients_.erase(clientfd);
    }

    close(clientfd);
    std::cout << "[Server] client disconnected fd=" << clientfd << std::endl;
}

void LoginServer::onMessage(int clientfd, const std::string& msg) {
    std::cout << "[Server] recv from " << clientfd << ": " << msg << std::endl;

    std::istringstream iss(msg);
    std::string cmd, user, pass;
    // iss >> cmd >> user >> pass;

    if (cmd == "LOGIN") {

        // if (user == "admin" && pass == "123456") {
        //     {
        //         std::lock_guard<std::mutex> lock(mutex_);
        //         clients_[clientfd] = user;
        //     }
        //     send(clientfd, "LOGIN_OK\n", 9, 0);
        // } else {
        //     send(clientfd, "LOGIN_FAIL\n", 11, 0);
        // }
    }
}
