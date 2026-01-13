//ClientEvent.h
#pragma once

#include <string>
#include "protocol_types.h"
#include <variant>

struct EvTcpConnected {};
struct EvTcpDisconnected {};
struct EvLoginOk    { proto::LoginResponse resp; };
struct EvLoginFail { proto::LoginResponse resp; };
struct EvOnlineUsers { proto::OnlineUsers users; };
struct EvUnknow { proto::Unknown error_msg; };
struct EvCmdConnect {
    std::string host;
    int port;
};
struct EvCmdDisconnect {};
struct EvCmdLogin {
    std::string user;
    std::string pass;
};

using ClientEvent = std::variant<
    // UI
    EvCmdConnect,
    EvCmdDisconnect,
    EvCmdLogin,

    // TCP
    EvTcpConnected,
    EvTcpDisconnected,

    // 协议
    EvLoginOk,
    EvLoginFail,
    EvOnlineUsers,
    EvUnknow
>;

