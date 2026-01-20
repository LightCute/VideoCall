//protocol_types.h
#pragma once
#include <string>
#include <vector>

namespace proto {

struct UserInfo {
    std::string username;
    int privilege = 0;
};

struct Unknown {
    std::string message;
};

struct LoginResponse {
    bool success = false;
    UserInfo user;
    std::string message;
};

struct OnlineUsers {
    std::vector<UserInfo> users;
};

// 新增：文本消息结构体（客户端→服务器）
struct SendTextMsg {
    std::string target_user;  // 目标用户名
    std::string content;      // 消息内容
    std::string from_user;    // 发送者用户名（服务器填充）
};

// 新增：转发文本消息结构体（服务器→客户端）
struct ForwardTextMsg {
    std::string from_user;    // 发送者用户名
    std::string content;      // 消息内容
};

}
