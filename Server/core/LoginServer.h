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
#include "./service/CallService.h"

class LoginServer {
public:
    LoginServer();
    ~LoginServer();
    bool start(int port);

private:
    void onAccept(int clientfd);
    void clientThread(int clientfd);
    void onMessage(int clientfd, const std::string& msg);
    void handleLogin(int fd, ServerEvent& event);
    void startHeartbeatMonitor();
    void handle(const SendLoginOk&);
    void handle(const SendLoginFail&);
    void handle(const BroadcastOnlineUsers&);
    void handle(const SendError&);
    void handle(const BroadcastLogout&);
    void handle(const SendHeartbeatAck& a);
    void handle(const UpdatePeerInfo&);
    void handle(const ForwardText&);          
    void handle(const SendUserNotFound&);     
    void handle(const SendCallIncoming& a);
    void handle(const SendCallAccepted& a);
    void handle(const SendCallRejected& a);
    CommandSocket listener_;
    ThreadPool pool_{8};
    SessionManager sessionMgr_;
    LoginService loginService_;
    CallService callService_;  // 新增：CallService成员
    ServerEventDispatcher dispatcher_; // 调整顺序：需在callService_之后初始化
};
