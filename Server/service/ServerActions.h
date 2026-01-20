// Service/ServerActions.h
#pragma once
#include <string>
#include <vector>
#include <map>
//#include "SessionManager.h"

// 单播：登录成功
struct SendLoginOk {
    int fd;
    std::string username;
    int privilege;
};

// 单播：登录失败
struct SendLoginFail {
    int fd;
    std::string reason;
};

struct SendError {
    int fd;
    std::string reason;
};

// 广播：在线用户列表
struct BroadcastOnlineUsers {
};

struct BroadcastLogout {
};

// 新增：服务器回应心跳
struct SendHeartbeatAck {
    int fd;
};

struct UpdatePeerInfo {
    int fd;
    std::string lanIp;
    std::string vpnIp;
    int udpPort;
};

struct ForwardText {
    int target_fd;               // 目标用户 FD（-1 表示用户不存在）
    std::string from_user;       // 发送者用户名
    std::string content;         // 消息内容
    int sender_fd;               // 发送者 FD（用于返回错误）
};

struct SendUserNotFound {
    int fd;                      // 发送者 FD
    std::string target_user;     // 不存在的用户名
};


// 未来可无限加
// struct KickUser { int fd; };
// struct ForceLogout { int fd; };
// struct SendChat { int from; std::string msg; };
