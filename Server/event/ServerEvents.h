//ServerEvents.h
#pragma once
#include <string>




namespace event {

struct ErrorEvent {
    std::string rawMsg;  // 收到的原始消息
    std::string reason;  // 解析失败原因
};

// 登录请求
struct LoginRequest {
    std::string username;
    std::string password;
};

// 以后可以非常自然地加
struct Logout {
};

struct Heartbeat {};  // 客户端心跳


struct Chat {
    std::string message;
};

} // namespace event
