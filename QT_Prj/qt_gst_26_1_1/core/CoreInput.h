// core/CoreInput.h
#pragma once
#include <variant>
#include <string>
namespace core {

struct EvCmdConnect { std::string host; int port; };
struct EvCmdDisconnect {};
struct EvCmdLogin { std::string user; std::string pass; };
struct EvTcpConnected {};
struct EvTcpDisconnected {};
struct EvLoginOk {};
struct EvLoginFail { std::string msg; };
struct EvOnlineUsers { std::string list; };
struct EvUnknow {};

using CoreInput = std::variant<
    EvCmdConnect,
    EvCmdDisconnect,
    EvCmdLogin,
    EvTcpConnected,
    EvTcpDisconnected,
    EvLoginOk,
    EvLoginFail,
    EvOnlineUsers,
    EvUnknow
    >;
}
