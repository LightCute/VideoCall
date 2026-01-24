//ServerEventDispatcher.h
#pragma once
#include "./event/ServerEvent.h"

#include "./service/LoginService.h"
#include "./session/SessionManager.h"
#include "./service/ServerAction.h"
#include "./service/ServerActions.h"
#include "./service/CallService.h"

class ServerEventDispatcher {
public:
    ServerEventDispatcher(LoginService& loginService,
                          SessionManager& sessionMgr,
                          CallService& callService);

    std::vector<ServerAction> dispatch(int fd, const ServerEvent& event);

private:
    std::vector<ServerAction> handle(int fd, const event::CallRequest& ev);
    std::vector<ServerAction> handle(int fd, const event::CallAccept& ev);
    std::vector<ServerAction> handle(int fd, const event::CallReject& ev);
    std::vector<ServerAction> handle(int fd, const event::LoginRequest& ev);
    std::vector<ServerAction> handle(int fd, const event::ErrorEvent& ev);
    std::vector<ServerAction> handle(int fd, const event::Logout& ev);
    std::vector<ServerAction> handle(int fd, const event::Heartbeat& ev);
    std::vector<ServerAction> handle(int fd, const event::RegisterPeer& ev);
    std::vector<ServerAction> handle(int fd, const event::SendTextToUser& ev);
    std::vector<ServerAction> handle(int fd, const event::MediaOffer& ev);   
    std::vector<ServerAction> handle(int fd, const event::MediaAnswer& ev);  
        //std::vector<ServerAction> handle(int fd, const event::Logout&)

    LoginService&  loginService_;
    SessionManager& sessionMgr_;
    CallService& callService_;
};
