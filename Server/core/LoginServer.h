//LoginServer.h
#pragma once
#include <thread>
#include <mutex>
#include <map>
#include <iostream>
#include <sstream>


#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "./util/ThreadPool.h"
#include "./net/PacketCodec.h"
#include "./net/CommandSocket.h"
#include "./event/ServerEvent.h"
#include "./event/ServerEventFactory.h"
#include "./session/SessionManager.h"
#include "./dispatcher/ServerEventDispatcher.h"
#include "./service/ServerAction.h"
#include "./protocol/protocol_text.h"
#include "./protocol/protocol_common.h"

class LoginServer {
public:
    LoginServer();
    bool start(int port);

private:
    void onAccept(int clientfd);
    void clientThread(int clientfd);
    void onMessage(int clientfd, const std::string& msg);
    void handleLogin(int fd, ServerEvent& event);
    void handleAction(const ServerAction& action);

    CommandSocket listener_;
    ThreadPool pool_{8};
    SessionManager sessionMgr_;
    ServerEventDispatcher dispatcher_;
    LoginService loginService_;
};
