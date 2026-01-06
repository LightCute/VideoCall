//ServerEventDispatcher.cpp
#include "./dispatcher/ServerEventDispatcher.h"



ServerEventDispatcher::ServerEventDispatcher(
    LoginService& loginService,
    SessionManager& sessionMgr)
    : loginService_(loginService),
      sessionMgr_(sessionMgr)
{}

ServerAction ServerEventDispatcher::dispatch(int fd,
                                             const ServerEvent& event) {
    ServerAction action;

    switch (event.type) {
    case ServerEventType::LoginRequest: {
        LoginResult result = loginService_.handleLogin(event.loginReq);

        action.type = ServerActionType::SendLoginResult;
        action.targetFd = fd;
        action.loginResult = result;

        if (result.success) {
            //sessionMgr_.onLoginSuccess(fd, result.username, result.privilege);

        }
        return action;
    }

    default:
        return {};
    }
}