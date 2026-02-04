// core/CoreOutput.h
#pragma once
#include <variant>
#include "ClientState.h"
#include "vector"
#include "CoreInput.h"
// 输出层事件全部纳入 core 命名空间
namespace core {

// ========== 第一步：明确分类，拆分出 3 种语义类型 ==========
// 1. UI 专属输出（仅用于通知 UI 更新状态/执行界面操作，无 IO 逻辑）
struct UiOutStateChanged {
    State from;
    State to;
};
struct UiOutLoginOk {};
struct UiOutLoginFail { std::string msg; };
struct UiOutDisconnected {};
struct UiOutOnlineUsers { std::vector<OnlineUser> list; };
struct UiOutForwardText { std::string from_user; std::string content; };
struct UiOutMediaReadyFinal {
    std::string peerIp;
    int peerPort;
};
struct UiOutShowIncomingCall { std::string from; };
struct UiOutCallEnded {
    std::string peer;
    std::string reason;
};
struct UiOutStopMedia {}; // 通知 UI 停止媒体层，属于 UI 职责

// 2. Executor 专属输出（仅用于向 Executor 下达 IO 命令，无 UI 逻辑）
struct ExecOutConnect { std::string host; int port; };
struct ExecOutSendLogin { std::string user; std::string pass; };
struct ExecOutSendPing {};
struct ExecOutSelectLan {};
struct ExecOutSelectVpn {};
struct ExecOutSendText { std::string target_user; std::string content; };
struct ExecOutSendCall { std::string target_user; };
struct ExecOutSendAcceptCall {};
struct ExecOutSendRejectCall {};
struct ExecOutSendMediaOffer { std::string peer; };
struct ExecOutSendMediaAnswer { std::string peer; };
struct ExecOutSendHangup {};
struct ExecOutLoginOk {}; // 仅用于通知 Executor 发送本地 IP，属于 IO 职责
struct ExecOutMediaReady { // 仅用于 Executor 选择 IP，不直接暴露给 UI
    std::string lanIp;
    std::string vpnIp;
    int peerPort;
};

// 3. 内部辅助类型（仅 Core 内部使用，不暴露给任何上层）
struct InternalOutUpdateAlive {};

// ========== 第二步：定义语义化的聚合类型 ==========
// UI 层仅能访问此类型
using UiOutput = std::variant<
    UiOutStateChanged,
    UiOutLoginOk,
    UiOutLoginFail,
    UiOutDisconnected,
    UiOutOnlineUsers,
    UiOutForwardText,
    UiOutShowIncomingCall,
    UiOutMediaReadyFinal,
    UiOutCallEnded,
    UiOutStopMedia
    >;

// Executor 层仅能访问此类型（Core 内部传递给 Executor）
using ExecOutput = std::variant<
    ExecOutConnect,
    ExecOutSendLogin,
    ExecOutSendPing,
    ExecOutSelectLan,
    ExecOutSelectVpn,
    ExecOutSendText,
    ExecOutSendCall,
    ExecOutSendAcceptCall,
    ExecOutSendRejectCall,
    ExecOutSendMediaOffer,
    ExecOutSendMediaAnswer,
    ExecOutSendHangup,
    ExecOutLoginOk,
    ExecOutMediaReady
    >;

// Core 内部统一 DSL（仅在 ClientCore、FSM 内部使用，不暴露给上层）
using CoreOutput = std::variant<
    UiOutput,
    ExecOutput,
    InternalOutUpdateAlive
    >;

} // namespace core
