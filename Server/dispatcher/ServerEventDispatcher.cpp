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



// 处理MediaOffer事件
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::MediaOffer& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证发送者是否已登录
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot send media offer"
        });
        return actions;
    }

    // 2. 获取发送者用户名和自身网络信息
    auto snapshot = sessionMgr_.snapshot();
    std::string from_user = snapshot.at(fd).user.username;
    ClientNetInfo from_net = snapshot.at(fd).net; // 发送者自己的地址

    // 3. 检查目标用户是否在线
    int target_fd = sessionMgr_.getFdByUsername(ev.target_user);
    if (target_fd == -1) {
        actions.emplace_back(SendUserNotFound{
            .fd = fd,
            .target_user = ev.target_user
        });
        return actions;
    }

    // 4. 查找通话会话（仅验证通话存在）
    auto call_session = callService_.onMediaNegotiate(from_user);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "No active call for media offer"
        });
        return actions;
    }

    // 5. 验证会话的目标用户是否匹配（防止跨会话协商）
    if (call_session->callee != ev.target_user && call_session->caller != ev.target_user) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "Media offer target mismatch with active call"
        });
        return actions;
    }

    // 6. 核心修改：将「发送者的地址」转发给「目标用户」（而非反向）
    actions.emplace_back(SendMediaOffer{
        .fd = target_fd,       // 发送给目标用户
        .peer = from_user,     // 发送者用户名
        .peer_net = from_net   // 发送者的网络地址
    });

    // 7. 标记发送者媒体就绪
    callService_.markMediaReady(from_user);

    std::cout << "[Dispatcher] Media offer from " << from_user << " forward to " << ev.target_user 
              << " lan=" << from_net.lanIp << " vpn=" << from_net.vpnIp << " port=" << from_net.udpPort << std::endl;
    return actions;
}


// 处理MediaAnswer事件
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::MediaAnswer& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证发送者是否已登录
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot send media answer"
        });
        return actions;
    }

    // 2. 获取发送者用户名和自身网络信息
    auto snapshot = sessionMgr_.snapshot();
    std::string from_user = snapshot.at(fd).user.username;
    ClientNetInfo from_net = snapshot.at(fd).net; // 发送者自己的地址

    // 3. 检查目标用户是否在线
    int target_fd = sessionMgr_.getFdByUsername(ev.target_user);
    if (target_fd == -1) {
        actions.emplace_back(SendUserNotFound{
            .fd = fd,
            .target_user = ev.target_user
        });
        return actions;
    }

    // 4. 查找通话会话（仅验证通话存在）
    auto call_session = callService_.onMediaNegotiate(from_user);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "No active call for media answer"
        });
        return actions;
    }

    // 5. 验证会话的目标用户是否匹配
    if (call_session->callee != ev.target_user && call_session->caller != ev.target_user) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "Media answer target mismatch with active call"
        });
        return actions;
    }

    // 6. 核心修改：将「发送者的地址」转发给「目标用户」
    actions.emplace_back(SendMediaAnswer{
        .fd = target_fd,       // 发送给目标用户
        .peer = from_user,     // 发送者用户名
        .peer_net = from_net   // 发送者的网络地址
    });

    // 7. 标记发送者媒体就绪
    callService_.markMediaReady(from_user);

    std::cout << "[Dispatcher] Media answer from " << from_user << " forward to " << ev.target_user 
              << " lan=" << from_net.lanIp << " vpn=" << from_net.vpnIp << " port=" << from_net.udpPort << std::endl;
    return actions;
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

// 简化：CALL_ENDED仅由服务端下发，客户端无需发送该指令，此处标记为废弃
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::CallEnded& ev)
{
    std::vector<ServerAction> actions;
    actions.emplace_back(SendError{
        .fd = fd,
        .reason = "Client cannot send CALL_ENDED, please use CALL_HANGUP instead"
    });
    std::cout << "[Dispatcher] Rejected client CALL_ENDED request from fd=" << fd << std::endl;
    return actions;
}

// 处理CallHangup事件（核心重构：收回通话结束裁决权）
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::CallHangup& ev)
{
    std::vector<ServerAction> actions;

    // 1. 验证当前用户是否已登录（防御式编程）
    if (!sessionMgr_.exists(fd)) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "You are not logged in, cannot hang up call"
        });
        return actions;
    }

    // 2. 获取当前用户（主动挂断方）的用户名
    auto snapshot = sessionMgr_.snapshot();
    auto fd_it = snapshot.find(fd);
    if (fd_it == snapshot.end()) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "User session not found"
        });
        return actions;
    }
    std::string current_user = fd_it->second.user.username;

    // 3. 双向查找当前用户的活跃通话会话（使用新增的findSessionByAnyUser）
    std::optional<CallSession> call_session = callService_.findSessionByAnyUser(current_user);
    if (!call_session) {
        actions.emplace_back(SendError{
            .fd = fd,
            .reason = "No active call to hang up"
        });
        return actions;
    }

    // 4. 确定对端用户名
    std::string peer_user = (call_session->caller == current_user) 
        ? call_session->callee 
        : call_session->caller;

    // 5. 查找对端用户的FD
    int peer_fd = sessionMgr_.getFdByUsername(peer_user);

    // 6. 步骤1：给对端发送CALL_ENDED（通知对方通话结束）
    if (peer_fd != -1) {
        actions.emplace_back(SendCallEnded{
            .fd = peer_fd,
            .peer = current_user,
            .reason = "Peer actively hung up the call"
        });
    }

    // 7. 步骤2：给自身发送CALL_ENDED（关键：让客户端FSM靠服务端通知结束，而非本地按钮）
    actions.emplace_back(SendCallEnded{
        .fd = fd,
        .peer = peer_user,
        .reason = "You hung up the call"
    });

    // 8. 步骤3：最后清理活跃通话会话（顺序不可乱，先通知再删除）
    callService_.deleteCallSessionByAnyUser(current_user);

    std::cout << "[Dispatcher] Call hung up by user: " << current_user 
              << ", peer: " << peer_user << std::endl;
    return actions;
}


// 处理 UserDisconnected 事件，接管业务裁决
std::vector<ServerAction>
ServerEventDispatcher::handle(int fd, const event::UserDisconnected& ev) {
    std::vector<ServerAction> actions;
    const std::string& username = ev.username;

    // 步骤1：裁决 - 判断用户是否处于活跃通话中（调用 CallService 的纯能力接口）
    if (!callService_.isInCall(username)) {
        // 无活跃通话，直接返回，不执行任何后续操作
        return actions;
    }

    // 步骤2：获取通话会话信息（调用 CallService 的纯能力接口）
    std::optional<CallSession> call_session = callService_.findSessionByAnyUser(username);
    if (!call_session) {
        return actions;
    }

    // 步骤3：确定对端用户名和 FD（调用 SessionManager 的查询能力）
    std::string peer_user = (call_session->caller == username) 
        ? call_session->callee 
        : call_session->caller;
    int peer_fd = sessionMgr_.getFdByUsername(peer_user);

    // 步骤4：生成 Action - 通知对端通话结束（业务执行）
    if (peer_fd != -1) {
        actions.emplace_back(SendCallEnded{
            .fd = peer_fd,
            .peer = username,
            .reason = "Peer disconnected unexpectedly"
        });
    }

    // 步骤5：清理通话会话（调用 CallService 的纯能力接口）
    callService_.deleteCallSessionByAnyUser(username);

    std::cout << "[Dispatcher] User disconnected and call ended: " << username 
              << ", peer: " << peer_user << std::endl;
    return actions;
}
