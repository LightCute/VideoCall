// core/CoreInput.h
#pragma once
#include <variant>
#include <string>
#include <vector>
namespace core {

struct OnlineUser {
    std::string name;
    int privilege;

    std::string lan_ip;   // 局域网 IP
    std::string vpn_ip;   // VPN IP（未来可用）
    int media_port;       // GStreamer/UDP 视频端口
};


struct InCmdSendText { std::string target_user; std::string content; };

struct InForwardText { std::string from_user; std::string content; };

// 所有 Core 输入事件前缀改为 In
struct InCmdConnect { std::string host; int port; };
struct InCmdDisconnect {};
struct InCmdLogin { std::string user; std::string pass; };
struct InTcpConnected {};
struct InTcpDisconnected {};
struct InLoginOk {};
struct InLoginFail { std::string msg; };
struct InOnlineUsers { std::vector<OnlineUser> users; };
struct InUnknow {};
struct InHeartbeatOk {};      // 收到 PONG
struct InHeartbeatTimeout {}; // 心跳超时
struct InHeartbeatTick {};    // Send PING
struct InSelectLan {};
struct InSelectVpn {};

// 发起呼叫
struct InCmdCall { std::string target_user; };
// 收到来电
struct InCallIncoming { std::string from; };
// 通话被接听
struct InCallAccepted { std::string peer; };
// 通话被拒绝
struct InCallRejected { std::string peer; };
// 用户接听/拒绝通话
struct InCmdAcceptCall {};
struct InCmdRejectCall {};
// 媒体协商响应
struct InMediaPeer {
    std::string peer;
    std::string lanIp;
    std::string vpnIp;
    int udpPort;
};



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
    InHeartbeatTick,
    InSelectLan,
    InSelectVpn,
    InCmdSendText,
    InForwardText,
    InCmdCall,
    InCallIncoming,
    InCallAccepted,
    InCallRejected,
    InCmdAcceptCall,
    InCmdRejectCall,
    InMediaPeer
    >;


inline std::string CoreInputIndexToName(size_t index) {
    // 严格按照CoreInput variant的类型顺序映射索引和名称
    switch (index) {
    case 0:  return "InCmdConnect";
    case 1:  return "InCmdDisconnect";
    case 2:  return "InCmdLogin";
    case 3:  return "InTcpConnected";
    case 4:  return "InTcpDisconnected";
    case 5:  return "InLoginOk";
    case 6:  return "InLoginFail";
    case 7:  return "InOnlineUsers";
    case 8:  return "InUnknow";
    case 9:  return "InHeartbeatOk";
    case 10: return "InHeartbeatTimeout";
    case 11: return "InHeartbeatTick";
    case 12: return "InSelectLan";
    case 13: return "InSelectVpn";
    case 14: return "InCmdSendText";
    case 15: return "InForwardText";
    case 16: return "InCmdCall";
    case 17: return "InCallIncoming";
    case 18: return "InCallAccepted";
    case 19: return "InCallRejected";
    case 20: return "InCmdAcceptCall";
    case 21: return "InCmdRejectCall";
    case 22: return "InMediaPeer";
    // 兜底：索引超出范围时返回提示（防止新增类型未更新函数）
    default: return "InUnknownCoreInputType";
    }
}

}
