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
    InForwardText
    >;
}
