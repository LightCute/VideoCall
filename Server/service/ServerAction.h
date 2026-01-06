// ServerAction.h
#pragma once
#include <string>
#include <vector>


enum class ServerActionType {
    None,
    SendLoginResult,
    BroadcastOnlineUsers
};


struct ServerAction {
    ServerActionType type = ServerActionType::None;
    int targetFd = -1;

    // 动作数据（不是协议字符串！）
    LoginResult loginResult;
};

// struct ServerAction {
//     ServerActionType type = ServerActionType::None;

//     int targetFd = -1;              // 单播
//     std::vector<int> targets;       // 广播
//     std::string payload;            // 已编码好的协议数据
// };
