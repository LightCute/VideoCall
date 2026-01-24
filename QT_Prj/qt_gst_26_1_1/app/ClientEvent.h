//ClientEvent.h
#pragma once

#include <string>
#include "protocol_types.h"
#include <variant>

// 协议层事件统一前缀：ProtoEvt
struct ProtoEvtTcpConnected {};
struct ProtoEvtTcpDisconnected {};
struct ProtoEvtLoginOk    { proto::LoginResponse resp; };
struct ProtoEvtLoginFail { proto::LoginResponse resp; };
struct ProtoEvtOnlineUsers { proto::OnlineUsers users; };
struct ProtoEvtUnknow { proto::Unknown error_msg; };
struct ProtoEvtCmdConnect {
    std::string host;
    int port;
};
struct ProtoEvtCmdDisconnect {};
struct ProtoEvtCmdLogin {
    std::string user;
    std::string pass;
};
struct ProtoEvHeartbeatAck{};

// 转发文本消息事件（服务器→客户端）
struct ProtoEvtForwardText {
    std::string from_user;  // 发送者用户名
    std::string content;    // 消息内容
};

// 来电事件
struct ProtoEvtCallIncoming {
    std::string from; // 来电方用户名
};

// 通话被接听事件
struct ProtoEvtCallAccepted {
    std::string peer; // 通话对方用户名
};

// 通话被拒绝事件
struct ProtoEvtCallRejected {
    std::string peer; // 拒绝方用户名
};

// 媒体协商响应事件
struct ProtoEvtMediaPeer {
    std::string peer;    // 对方用户名
    std::string lanIp;   // 对方局域网IP
    std::string vpnIp;   // 对方VPN IP
    int udpPort;         // 对方媒体UDP端口
};

using ClientEvent = std::variant<
    // UI 触发的协议命令
    ProtoEvtCmdConnect,
    ProtoEvtCmdDisconnect,
    ProtoEvtCmdLogin,

    // TCP 底层事件
    ProtoEvtTcpConnected,
    ProtoEvtTcpDisconnected,

    // 协议解析后的业务事件
    ProtoEvtLoginOk,
    ProtoEvtLoginFail,
    ProtoEvtOnlineUsers,
    ProtoEvtUnknow,
    ProtoEvHeartbeatAck,
    ProtoEvtForwardText,
    ProtoEvtCallIncoming,
    ProtoEvtCallAccepted,
    ProtoEvtCallRejected,
    ProtoEvtMediaPeer
    >;
