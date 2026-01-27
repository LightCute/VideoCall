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
struct OutSelectLan {};
struct OutSelectVpn {};
struct OutSendText { std::string target_user; std::string content; };
struct OutForwardText { std::string from_user; std::string content; };
// 发送呼叫请求
struct OutSendCall { std::string target_user; };
// 发送接听/拒绝请求
struct OutSendAcceptCall {};
struct OutSendRejectCall {};
// 发送媒体Offer/Answer
struct OutSendMediaOffer { std::string peer; };
struct OutSendMediaAnswer { std::string peer; };
// 媒体信息就绪
struct OutMediaReady {
    std::string lanIp;
    std::string vpnIp;
    int peerPort;
};
struct OutMediaReadyFinal {
    std::string peerIp;
    int peerPort;
};

// 来电通知UI
struct OutShowIncomingCall { std::string from; };

using CoreOutput = std::variant<
    OutStateChanged,
    OutLoginOk,
    OutLoginFail,
    OutDisconnected,
    OutConnect,
    OutSendLogin,
    OutSendPing,
    OutUpdateAlive,
    OutOnlineUsers,
    OutSelectLan,
    OutSelectVpn,
    OutSendText,
    OutForwardText,
    OutSendCall,
    OutSendAcceptCall,
    OutSendRejectCall,
    OutSendMediaOffer,
    OutSendMediaAnswer,
    OutMediaReady,
    OutShowIncomingCall,
    OutMediaReadyFinal
    >;

} // namespace core
