//ClientEventEnum.h
#pragma once


enum class EventType {
    // ===== UI 命令 =====
    CmdConnect,
    CmdDisconnect,
    CmdLogin,

    // ===== TCP =====
    TcpConnected,
    TcpDisconnected,

    // ===== 协议 =====
    LoginOk,
    LoginFail,
    OnlineUsers,
    Unknow
};
