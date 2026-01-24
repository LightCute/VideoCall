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
    Unknow,

    HeartbeatOk,
    HeartbeatTimeout,
    HeartbeatTick,

    SelectLan,
    SelectVpn,
    CmdSendText,
    ForwardText,
    CmdCall,
    CallIncoming,
    CallAccepted,
    CallRejected,
    CmdAcceptCall,
    CmdRejectCall,
    MediaPeer
    // CallIncoming,    // 收到来电
    // CallAccepted,    // 通话被接听
    // CallRejected,    // 通话被拒绝
    // MediaOfferResp,  // 媒体Offer响应
    // MediaAnswerResp  // 媒体Answer响应
};
