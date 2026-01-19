// core/CoreOutput.h
#pragma once
#include <variant>
#include "ClientState.h"
#include "vector"
#include "CoreInput.h"
// 输出层事件全部纳入 core 命名空间
namespace core {

struct OutStateChanged {
    State from;
    State to;
};
struct OutLoginOk {};
struct OutLoginFail { std::string msg; };
struct OutDisconnected {};
struct OutConnect { std::string host; int port; };
struct OutSendLogin { std::string user; std::string pass; };
struct OutSendPing {};   // Core -> Executor：发送心跳
struct OutUpdateAlive {};
struct OutOnlineUsers { std::vector<OnlineUser> list; };



using CoreOutput = std::variant<
    OutStateChanged,
    OutLoginOk,
    OutLoginFail,
    OutDisconnected,
    OutConnect,
    OutSendLogin,
    OutSendPing,
    OutUpdateAlive,
    OutOnlineUsers
    >;

} // namespace core
