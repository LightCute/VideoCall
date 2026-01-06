//ServerEventDispatcher.h
#pragma once
#include "./event/ServerEvent.h"

#include "./service/LoginService.h"
#include "./session/SessionManager.h"
#include "./service/ServerAction.h"

using namespace proto;

class ServerEventDispatcher {
public:
    ServerEventDispatcher(LoginService& loginService,
                          SessionManager& sessionMgr);

    ServerAction dispatch(int fd, const ServerEvent& event);

private:
    LoginService&  loginService_;
    SessionManager& sessionMgr_;
};
