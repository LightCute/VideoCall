//ClientEventEnum.h
#pragma once
#include <string>

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


inline std::string EventTypeToString(EventType type) {
    // 使用switch-case精准映射每个枚举值
    switch (type) {
    case EventType::CmdConnect:        return "InCmdConnect";
    case EventType::CmdDisconnect:     return "InCmdDisconnect";
    case EventType::CmdLogin:          return "InCmdLogin";
    case EventType::TcpConnected:      return "InTcpConnected";
    case EventType::TcpDisconnected:   return "InTcpDisconnected";
    case EventType::LoginOk:           return "InLoginOk";
    case EventType::LoginFail:         return "InLoginFail";
    case EventType::OnlineUsers:       return "InOnlineUsers";
    case EventType::Unknow:            return "InUnknow";
    case EventType::HeartbeatOk:       return "InHeartbeatOk";
    case EventType::HeartbeatTimeout:  return "InHeartbeatTimeout";
    case EventType::HeartbeatTick:     return "InHeartbeatTick";
    case EventType::SelectLan:         return "InSelectLan";
    case EventType::SelectVpn:         return "InSelectVpn";
    case EventType::CmdSendText:       return "InCmdSendText";
    case EventType::ForwardText:       return "InForwardText";
    case EventType::CmdCall:           return "InCmdCall";
    case EventType::CallIncoming:      return "InCallIncoming";
    case EventType::CallAccepted:      return "InCallAccepted";
    case EventType::CallRejected:      return "InCallRejected";
    case EventType::CmdAcceptCall:     return "InCmdAcceptCall";
    case EventType::CmdRejectCall:     return "InCmdRejectCall";
    case EventType::MediaPeer:         return "InMediaPeer";

    // 处理未匹配到的枚举值（健壮性保障）
    default: return "InUnknownEventType";
    }
}
