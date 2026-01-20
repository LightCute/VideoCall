//ServerEventDispatcher.cpp
#include "./dispatcher/ServerEventDispatcher.h"
#include "iostream"


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
    if (!sessionMgr_.exists(fd)) {
        std::cout << "[Server] Ignore heartbeat before login, fd=" << fd << std::endl;
        return {};   // ❗ 不回 PONG
    }

    sessionMgr_.updateHeartbeat(fd);
    return {SendHeartbeatAck{ .fd = fd }};
}

std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::RegisterPeer& ev)
{
    std::cout << "[Dispatcher] RegisterPeer from fd=" << fd
              << " lan=" << ev.lanIp
              << " vpn=" << ev.vpnIp
              << " port=" << ev.udpPort << std::endl;

    return {
        UpdatePeerInfo{
            .fd = fd,
            .lanIp = ev.lanIp,
            .vpnIp = ev.vpnIp,
            .udpPort = ev.udpPort
        }
    };
}


std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::SendTextToUser& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证发送者是否在线
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot send message"
        });
        return actions;
    }

    // 2. 获取发送者用户名（从 Session 中读取）
    auto snapshot = sessionMgr_.snapshot();
    std::string from_user = snapshot.at(fd).user.username;

    // 3. 根据目标用户名查找 FD
    int target_fd = sessionMgr_.getFdByUsername(ev.target_user);

    if (target_fd == -1) {
        // 目标用户不存在：返回错误
        actions.emplace_back(SendUserNotFound{
            .fd = fd,
            .target_user = ev.target_user
        });
    } else {
        // 目标用户存在：生成定向转发 Action
        actions.emplace_back(ForwardText{
            .target_fd = target_fd,
            .from_user = from_user,
            .content = ev.content,
            .sender_fd = fd
        });
    }

    std::cout << "[Dispatcher] SendTextToUser from fd=" << fd 
              << " user=" << from_user 
              << " target=" << ev.target_user 
              << " content=" << ev.content << std::endl;

    return actions;
}
