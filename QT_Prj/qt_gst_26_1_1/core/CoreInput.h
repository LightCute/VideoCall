// core/CoreInput.h
#pragma once
#include <variant>
#include <string>
namespace core {

// 所有 Core 输入事件前缀改为 In
struct InCmdConnect { std::string host; int port; };
struct InCmdDisconnect {};
struct InCmdLogin { std::string user; std::string pass; };
struct InTcpConnected {};
struct InTcpDisconnected {};
struct InLoginOk {};
struct InLoginFail { std::string msg; };
struct InOnlineUsers { std::string list; };
struct InUnknow {};
struct InHeartbeatOk {};      // 收到 PONG
struct InHeartbeatTimeout {}; // 心跳超时
struct InHeartbeatTick {};    // Send PING

using CoreInput = std::variant<
    InCmdConnect,
    InCmdDisconnect,
    InCmdLogin,
    InTcpConnected,
    InTcpDisconnected,
    InLoginOk,
    InLoginFail,
    InOnlineUsers,
    InUnknow,
    InHeartbeatOk,
    InHeartbeatTimeout,
    InHeartbeatTick
    >;
}
