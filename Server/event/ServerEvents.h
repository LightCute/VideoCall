//ServerEvents.h
#pragma once
#include <string>




namespace event {

struct SendTextToUser {
    std::string target_user;  // 客户端指定的目标用户名
    std::string content;      // 消息内容
    std::string from_user;    // 发送者用户名（服务端从 Session 中补全）
};

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

struct RegisterPeer {
    std::string lanIp;
    std::string vpnIp;
    int udpPort;
};


struct Chat {
    std::string message;
};

struct CallRequest {
    std::string target_user;   // 呼叫的目标用户名
};

struct CallAccept {};  // 接受通话（无需带target，由CallSession维护）
struct CallReject {};  // 拒绝通话（同理）

// 媒体协商相关事件
struct MediaOffer {
    std::string target_user;  // 协商的目标用户
};

struct MediaAnswer {
    std::string target_user;  // 协商的目标用户
};


} // namespace event
