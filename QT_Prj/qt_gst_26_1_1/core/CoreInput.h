// core/CoreInput.h
#pragma once
#include "ClientEvent.h"
using CoreInput = std::variant<
    // UI 命令
    EvCmdConnect,
    EvCmdDisconnect,
    EvCmdLogin,

    // TCP
    EvTcpConnected,
    EvTcpDisconnected,

    // 协议（来自网络）
    EvLoginOk,
    EvLoginFail,
    EvOnlineUsers,
    EvUnknow
    >;
