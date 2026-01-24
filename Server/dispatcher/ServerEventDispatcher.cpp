//ServerEventDispatcher.cpp
#include "./dispatcher/ServerEventDispatcher.h"
#include "iostream"


ServerEventDispatcher::ServerEventDispatcher(
    LoginService& loginService,
    SessionManager& sessionMgr,
    CallService& callService)
    : loginService_(loginService),
      sessionMgr_(sessionMgr),
      callService_(callService)
{}


std::vector<ServerAction>
ServerEventDispatcher::dispatch(int fd, const ServerEvent& event)
{
    return std::visit([&](auto&& ev) {
        return handle(fd, ev);
    }, event);
}


std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::CallRequest& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证发送者是否已登录
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot make call"
        });
        return actions;
    }

    // 2. 获取发送者用户名
    auto snapshot = sessionMgr_.snapshot();
    std::string from_user = snapshot.at(fd).user.username;

    // 3. 检查目标用户是否在线
    int target_fd = sessionMgr_.getFdByUsername(ev.target_user);
    if (target_fd == -1) {
        actions.emplace_back(SendUserNotFound{
            .fd = fd,
            .target_user = ev.target_user
        });
        return actions;
    }

    // 4. 调用CallService创建通话会话
    auto call_session = callService_.onCallRequest(from_user, ev.target_user);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "Failed to make call: target user is busy"
        });
        return actions;
    }

    // 5. 生成Action：通知被呼叫方有新来电
    actions.emplace_back(SendCallIncoming{
        .target_fd = target_fd,
        .from_user = from_user
    });

    std::cout << "[Dispatcher] Call request from " << from_user << " to " << ev.target_user << std::endl;
    return actions;
}

// 处理接受通话事件
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::CallAccept& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证用户是否已登录
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot accept call"
        });
        return actions;
    }

    // 2. 获取当前用户（被呼叫方）用户名
    auto snapshot = sessionMgr_.snapshot();
    std::string callee = snapshot.at(fd).user.username;

    // 3. 调用CallService处理接受通话
    auto call_session = callService_.onAccept(callee);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "No incoming call to accept"
        });
        return actions;
    }

    // 4. 获取呼叫方的FD
    int caller_fd = sessionMgr_.getFdByUsername(call_session->caller);
    if (caller_fd == -1) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "Caller is offline"
        });
        return actions;
    }

    // 5. 生成Action：通知呼叫方和被呼叫方通话已接通
    // 通知被呼叫方
    actions.emplace_back(SendCallAccepted{
        .fd = fd,
        .peer = call_session->caller
    });
    // 通知呼叫方
    actions.emplace_back(SendCallAccepted{
        .fd = caller_fd,
        .peer = callee
    });

    std::cout << "[Dispatcher] Call accepted: " << call_session->caller << " <-> " << callee << std::endl;
    return actions;
}

// 处理拒绝通话事件
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::CallReject& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证用户是否已登录
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot reject call"
        });
        return actions;
    }

    // 2. 获取当前用户（被呼叫方）用户名
    auto snapshot = sessionMgr_.snapshot();
    std::string callee = snapshot.at(fd).user.username;

    // 3. 调用CallService处理拒绝通话
    auto call_session = callService_.onReject(callee);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "No incoming call to reject"
        });
        return actions;
    }

    // 4. 获取呼叫方的FD
    int caller_fd = sessionMgr_.getFdByUsername(call_session->caller);
    if (caller_fd != -1) {
        // 生成Action：通知呼叫方通话被拒绝
        actions.emplace_back(SendCallRejected{
            .fd = caller_fd,
            .peer = callee
        });
    }

    std::cout << "[Dispatcher] Call rejected: " << call_session->caller << " -> " << callee << std::endl;
    return actions;
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
