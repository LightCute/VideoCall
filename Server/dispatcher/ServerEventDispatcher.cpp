//ServerEventDispatcher.cpp
#include "./dispatcher/ServerEventDispatcher.h"



ServerEventDispatcher::ServerEventDispatcher(
    LoginService& loginService,
    SessionManager& sessionMgr)
    : loginService_(loginService),
      sessionMgr_(sessionMgr)
{}


std::vector<ServerAction>
ServerEventDispatcher::dispatch(int fd, const ServerEvent& event)
{
    return std::visit([&](auto&& ev) {
        return handle(fd, ev);
    }, event);
}


// 登录
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::LoginRequest& ev)
{
    std::vector<ServerAction> actions;

    LoginResult r = loginService_.handleLogin(ev);

    if (r.success) {
        sessionMgr_.login(fd, {
            domain::User{ r.username, r.privilege },
            true
        });

        actions.emplace_back(SendLoginOk{
            .fd = fd,
            .username = r.username,
            .privilege = r.privilege
        });

        actions.emplace_back(BroadcastOnlineUsers{
            
        });
    } else {
        actions.emplace_back(SendLoginFail{
            .fd = fd,
            .reason = r.reason
        });
    }

    return actions;
}


std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::ErrorEvent& ev)
{
    std::vector<ServerAction> actions;
    
    actions.emplace_back(SendError{
        .fd = fd,
        .reason = ev.reason +ev.rawMsg
    });

    return actions;
}

std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::Logout& ev)
{
    std::vector<ServerAction> actions;

    // 关键：先从 SessionManager 里移除
    sessionMgr_.logout(fd);

    // 然后广播新的在线列表
    actions.emplace_back(BroadcastOnlineUsers{});

    return actions;
}

std::vector<ServerAction> ServerEventDispatcher::handle(int fd, const event::Heartbeat&) {
    sessionMgr_.updateHeartbeat(fd);  // 更新时间戳
    return {}; // 心跳不触发其他动作
}
