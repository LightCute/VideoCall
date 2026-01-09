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
    std::map<int, ClientInfo> snapshot;
};

// 未来可无限加
// struct KickUser { int fd; };
// struct ForceLogout { int fd; };
// struct SendChat { int from; std::string msg; };
