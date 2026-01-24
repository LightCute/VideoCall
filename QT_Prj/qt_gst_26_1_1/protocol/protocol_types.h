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

// 文本消息结构体（客户端→服务器）
struct SendTextMsg {
    std::string target_user;  // 目标用户名
    std::string content;      // 消息内容
    std::string from_user;    // 发送者用户名（服务器填充）
};

// 转发文本消息结构体（服务器→客户端）
struct ForwardTextMsg {
    std::string from_user;    // 发送者用户名
    std::string content;      // 消息内容
};

// 来电通知结构体
struct CallIncoming {
    std::string from; // 来电方用户名
};

// 通话被接听结构体
struct CallAccepted {
    std::string peer; // 通话对方用户名
};

// 通话被拒绝结构体
struct CallRejected {
    std::string peer; // 拒绝方用户名
};

// 媒体协商响应结构体
struct MediaPeerResp {
    std::string peer;    // 对方用户名
    std::string lanIp;   // 对方局域网IP
    std::string vpnIp;   // 对方VPN IP
    int udpPort;         // 对方媒体UDP端口
};

}
