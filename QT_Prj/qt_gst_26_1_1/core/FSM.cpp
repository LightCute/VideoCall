// core/FSM.cpp
#include "FSM.h"
#include "CoreOutput.h"
#include <set>
#include <iostream>

FSM::FSM() {
    initTable();
}

bool FSM::isOnlineState(State s) {
    return s != State::Disconnected;
}

// 补充 core:: 前缀，返回符合新语义规范的 CoreOutput 列表
std::vector<core::CoreOutput>
FSM::handle(State current, const core::CoreInput& ev)
{
    EventType evType = eventTypeFromInput(ev);
    std::cout << "[FSM] Current state: " << stateToString(current)
              << ", Received event type: " << EventTypeToString(evType) << std::endl;
    if (isOnlineState(current) && evType == EventType::HeartbeatTick) {
        // 直接返回，不走 FSM 表：OutSendPing 对应 ExecOutSendPing，包裹进 ExecOutput
        std::cout << "[FSM] Online state, directly handle HeartbeatTick event" << std::endl;
        core::ExecOutSendPing execSendPing;
        return { core::ExecOutput{execSendPing} };
    }

    else if (isOnlineState(current) && evType == EventType::HeartbeatOk) {
        // 直接返回，不走 FSM 表：OutUpdateAlive 对应 InternalOutUpdateAlive，直接作为 CoreOutput
        core::InternalOutUpdateAlive internalUpdateAlive;
        return { internalUpdateAlive };
    }

    if (isOnlineState(current)) {
        // 定义需要统一处理的掉线事件类型
        std::set<EventType> disconnectEvents = {
            EventType::TcpDisconnected,  // TCP链路断开
            EventType::HeartbeatTimeout   // 心跳超时（触发主动断线）
        };

        if (disconnectEvents.count(evType) > 0) {
            std::cout << "[FSM] Online state, directly handle disconnect event: " << EventTypeToString(evType) << std::endl;
            // 统一返回掉线输出，所有在线状态掉线都切换到Disconnected
            // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
            // 2. 掉线通知：UiOutDisconnected 包裹进 UiOutput
            std::vector<core::CoreOutput> out;
            core::UiOutStateChanged uiStateChange{current, State::Disconnected};
            core::UiOutDisconnected uiDisconnected;
            out.push_back(core::UiOutput{uiStateChange});
            out.push_back(core::UiOutput{uiDisconnected});
            return out;
        }
    }

    for (auto& entry : table_) {
        if (entry.current_state == current &&
            entry.event_type == evType) {

            auto outputs = entry.action(current, ev);
            return outputs;
        }
    }
    return {};
}

EventType FSM::eventTypeFromInput(const core::CoreInput& ev) {
    EventType evType = EventType::Unknow;
    std::visit([&evType](auto&& e){
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, core::InCmdConnect>)      evType = EventType::CmdConnect;
        else if constexpr (std::is_same_v<T, core::InCmdDisconnect>) evType = EventType::CmdDisconnect;
        else if constexpr (std::is_same_v<T, core::InCmdLogin>)   evType = EventType::CmdLogin;

        else if constexpr (std::is_same_v<T, core::InTcpConnected>)    evType = EventType::TcpConnected;
        else if constexpr (std::is_same_v<T, core::InTcpDisconnected>) evType = EventType::TcpDisconnected;

        else if constexpr (std::is_same_v<T, core::InLoginOk>)   evType = EventType::LoginOk;
        else if constexpr (std::is_same_v<T, core::InLoginFail>) evType = EventType::LoginFail;
        else if constexpr (std::is_same_v<T, core::InOnlineUsers>) evType = EventType::OnlineUsers;

        else if constexpr (std::is_same_v<T, core::InHeartbeatOk>)
            evType = EventType::HeartbeatOk;

        else if constexpr (std::is_same_v<T, core::InHeartbeatTimeout>)
            evType = EventType::HeartbeatTimeout;

        else if constexpr (std::is_same_v<T, core::InHeartbeatTick>)
            evType = EventType::HeartbeatTick;

        else if constexpr (std::is_same_v<T, core::InSelectLan>)
            evType = EventType::SelectLan;

        else if constexpr (std::is_same_v<T, core::InSelectVpn>)
            evType = EventType::SelectVpn;

        else if constexpr (std::is_same_v<T, core::InCmdSendText>)
            evType = EventType::CmdSendText;

        else if constexpr (std::is_same_v<T, core::InForwardText>)
            evType = EventType::ForwardText;

        else if constexpr (std::is_same_v<T, core::InCmdCall>)
            evType = EventType::CmdCall;
        else if constexpr (std::is_same_v<T, core::InCallIncoming>)
            evType = EventType::CallIncoming;
        else if constexpr (std::is_same_v<T, core::InCallAccepted>)
            evType = EventType::CallAccepted;
        else if constexpr (std::is_same_v<T, core::InCallRejected>)
            evType = EventType::CallRejected;
        else if constexpr (std::is_same_v<T, core::InCmdAcceptCall>)
            evType = EventType::CmdAcceptCall;
        else if constexpr (std::is_same_v<T, core::InCmdRejectCall>)
            evType = EventType::CmdRejectCall;
        else if constexpr (std::is_same_v<T, core::InMediaPeer>)
            evType = EventType::MediaPeer;
        else if constexpr (std::is_same_v<T, core::InCmdHangup>)
            evType = EventType::CmdHangup;
        else if constexpr (std::is_same_v<T, core::InCallEnded>)
            evType = EventType::CallEnded;

        else evType = EventType::Unknow;
    }, ev);
    return evType;
}

void FSM::initTable() {
    table_ = {
        // ===== 断开态 =====
        { State::Disconnected, EventType::CmdConnect,
            // 补充 core:: 前缀，按新语义规范构建输出
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 替换为 InCmdConnect，构建 UiOutput 和 ExecOutput
                if (auto e = std::get_if<core::InCmdConnect>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::Connecting};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 连接命令：ExecOutConnect 包裹进 ExecOutput
                    core::ExecOutConnect execConnect{e->host, e->port};
                    out.push_back(core::ExecOutput{execConnect});
                }
                return out;
            },
            State::Connecting
        },

        // ===== 正在连接 =====
        { State::Connecting, EventType::TcpConnected,
            // 补充 core:: 前缀，按新语义规范构建输出
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 替换为 InTcpConnected，构建 UiOutput（状态变更）
                if (auto e = std::get_if<core::InTcpConnected>(&ev)) {
                    core::UiOutStateChanged uiStateChange{cur, State::Connected};
                    out.push_back(core::UiOutput{uiStateChange});
                }
                return out;
            },
            State::Connected
        },

        { State::Connecting, EventType::TcpDisconnected,
            // 补充 core:: 前缀，无输出（空列表）
            [](State cur, const core::CoreInput&){ return std::vector<core::CoreOutput>{}; },
            State::Disconnected
        },

        // ===== 已连接 =====
        { State::Connected, EventType::CmdLogin,
            // 补充 core:: 前缀，按新语义规范构建输出
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 替换为 InCmdLogin，构建 UiOutput 和 ExecOutput
                if (auto e = std::get_if<core::InCmdLogin>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::LoggingIn};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 发送登录命令：ExecOutSendLogin 包裹进 ExecOutput
                    core::ExecOutSendLogin execSendLogin{e->user, e->pass};
                    out.push_back(core::ExecOutput{execSendLogin});
                }
                return out;
            },
            State::LoggingIn
        },

        // ===== 正在登录 =====
        { State::LoggingIn, EventType::LoginOk ,
            // 补充 core:: 前缀，按新语义规范构建输出
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 替换为 InLoginOk，构建 ExecOutput 和两个 UiOutput
                if (auto e = std::get_if<core::InLoginOk>(&ev)) {
                    // 1. 通知 Executor 发送本地 IP：ExecOutLoginOk 包裹进 ExecOutput
                    core::ExecOutLoginOk execLoginOk;
                    out.push_back(core::ExecOutput{execLoginOk});

                    // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 3. 通知 UI 登录成功：UiOutLoginOk 包裹进 UiOutput
                    core::UiOutLoginOk uiLoginOk;
                    out.push_back(core::UiOutput{uiLoginOk});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggingIn, EventType::LoginFail ,
            // 补充 core:: 前缀，按新语义规范构建输出
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 替换为 InLoginFail，构建 UiOutput（登录失败+状态变更）
                if (auto e = std::get_if<core::InLoginFail>(&ev)) {
                    // 1. 通知 UI 登录失败：UiOutLoginFail 包裹进 UiOutput
                    core::UiOutLoginFail uiLoginFail{e->msg};
                    out.push_back(core::UiOutput{uiLoginFail});

                    // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::Connected};
                    out.push_back(core::UiOutput{uiStateChange});
                }
                return out;
            },
            State::Connected
        },

        { State::LoggedIn, EventType::OnlineUsers,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InOnlineUsers>(&ev)) {
                    // 通知 UI 更新在线用户列表：UiOutOnlineUsers 包裹进 UiOutput
                    core::UiOutOnlineUsers uiOnlineUsers{e->users};
                    out.push_back(core::UiOutput{uiOnlineUsers});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::SelectLan,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InSelectLan>(&ev)) {
                    // 通知 Executor 选择 LAN：ExecOutSelectLan 包裹进 ExecOutput
                    core::ExecOutSelectLan execSelectLan;
                    out.push_back(core::ExecOutput{execSelectLan});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::SelectVpn,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InSelectVpn>(&ev)) {
                    // 通知 Executor 选择 VPN：ExecOutSelectVpn 包裹进 ExecOutput
                    core::ExecOutSelectVpn execSelectVpn;
                    out.push_back(core::ExecOutput{execSelectVpn});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::CmdSendText,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCmdSendText>(&ev)) {
                    // 通知 Executor 发送文本：ExecOutSendText 包裹进 ExecOutput
                    core::ExecOutSendText execSendText{e->target_user, e->content};
                    out.push_back(core::ExecOutput{execSendText});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::ForwardText,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InForwardText>(&ev)) {
                    // 通知 UI 显示转发文本：UiOutForwardText 包裹进 UiOutput
                    core::UiOutForwardText uiForwardText{e->from_user, e->content};
                    out.push_back(core::UiOutput{uiForwardText});
                }
                return out;
            },
            State::LoggedIn
        },

        // 1. LoggedIn状态下发起呼叫
        { State::LoggedIn, EventType::CmdCall,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCmdCall>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::CALLING};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 Executor 发送呼叫：ExecOutSendCall 包裹进 ExecOutput
                    core::ExecOutSendCall execSendCall{e->target_user};
                    out.push_back(core::ExecOutput{execSendCall});
                }
                return out;
            },
            State::CALLING
        },

        // 2. LoggedIn状态下收到来电
        { State::LoggedIn, EventType::CallIncoming,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCallIncoming>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::RINGING};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 UI 显示来电：UiOutShowIncomingCall 包裹进 UiOutput
                    core::UiOutShowIncomingCall uiShowIncomingCall{e->from};
                    out.push_back(core::UiOutput{uiShowIncomingCall});
                }
                return out;
            },
            State::RINGING
        },

        // 3. RINGING状态下用户接听
        { State::RINGING, EventType::CmdAcceptCall,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCmdAcceptCall>(&ev))
                {
                    // 通知 Executor 发送接听命令：ExecOutSendAcceptCall 包裹进 ExecOutput
                    core::ExecOutSendAcceptCall execSendAcceptCall;
                    out.push_back(core::ExecOutput{execSendAcceptCall});
                }
                return out;
            },
            State::RINGING
        },

        // 4. RINGING状态下用户拒绝
        { State::RINGING, EventType::CmdRejectCall,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCmdRejectCall>(&ev))
                {
                    // 1. 通知 Executor 发送拒绝命令：ExecOutSendRejectCall 包裹进 ExecOutput
                    core::ExecOutSendRejectCall execSendRejectCall;
                    out.push_back(core::ExecOutput{execSendRejectCall});

                    // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                    out.push_back(core::UiOutput{uiStateChange});
                }
                return out;
            },
            State::LoggedIn
        },

        // 5. CALLING/RINGING状态下收到通话被接听
        { State::CALLING, EventType::CallAccepted,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCallAccepted>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::IN_CALL};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 Executor 发送媒体 Offer：ExecOutSendMediaOffer 包裹进 ExecOutput
                    core::ExecOutSendMediaOffer execSendMediaOffer{e->peer};
                    out.push_back(core::ExecOutput{execSendMediaOffer});
                }
                return out;
            },
            State::IN_CALL
        },

        { State::CALLING, EventType::MediaPeer,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InMediaPeer>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::MEDIA_READY};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 Executor 媒体就绪（选择 IP）：ExecOutMediaReady 包裹进 ExecOutput
                    core::ExecOutMediaReady execMediaReady{e->lanIp, e->vpnIp, e->udpPort};
                    out.push_back(core::ExecOutput{execMediaReady});

                    // 3. 通知 Executor 发送媒体 Answer：ExecOutSendMediaAnswer 包裹进 ExecOutput
                    core::ExecOutSendMediaAnswer execSendMediaAnswer{e->peer};
                    out.push_back(core::ExecOutput{execSendMediaAnswer});
                }
                return out;
            },
            State::MEDIA_READY
        },

        { State::RINGING, EventType::CallAccepted,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCallAccepted>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::IN_CALL};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 Executor 发送媒体 Offer：ExecOutSendMediaOffer 包裹进 ExecOutput
                    core::ExecOutSendMediaOffer execSendMediaOffer{e->peer};
                    out.push_back(core::ExecOutput{execSendMediaOffer});
                }
                return out;
            },
            State::IN_CALL
        },

        // 6. CALLING状态下收到通话被拒绝
        { State::CALLING, EventType::CallRejected,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});
                return out;
            },
            State::LoggedIn
        },

        // 7. IN_CALL状态下收到媒体信息
        { State::IN_CALL, EventType::MediaPeer,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InMediaPeer>(&ev)) {
                    // 1. 状态变更：UiOutStateChanged 包裹进 UiOutput
                    core::UiOutStateChanged uiStateChange{cur, State::MEDIA_READY};
                    out.push_back(core::UiOutput{uiStateChange});

                    // 2. 通知 Executor 媒体就绪（选择 IP）：ExecOutMediaReady 包裹进 ExecOutput
                    core::ExecOutMediaReady execMediaReady{e->lanIp, e->vpnIp, e->udpPort};
                    out.push_back(core::ExecOutput{execMediaReady});
                }
                return out;
            },
            State::MEDIA_READY
        },

        // ===== 新增：主动挂断（CmdHangup）=====
        // 1. CALLING状态下主动挂断
        { State::CALLING, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 1. 通知 Executor 发送挂断命令：ExecOutSendHangup 包裹进 ExecOutput
                core::ExecOutSendHangup execSendHangup;
                out.push_back(core::ExecOutput{execSendHangup});

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                return out;
            },
            State::LoggedIn
        },

        // 2. RINGING状态下主动挂断
        { State::RINGING, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 1. 通知 Executor 发送挂断命令：ExecOutSendHangup 包裹进 ExecOutput
                core::ExecOutSendHangup execSendHangup;
                out.push_back(core::ExecOutput{execSendHangup});

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                return out;
            },
            State::LoggedIn
        },

        // 3. IN_CALL状态下主动挂断
        { State::IN_CALL, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 1. 通知 Executor 发送挂断命令：ExecOutSendHangup 包裹进 ExecOutput
                core::ExecOutSendHangup execSendHangup;
                out.push_back(core::ExecOutput{execSendHangup});

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                return out;
            },
            State::LoggedIn
        },

        // 4. MEDIA_READY状态下主动挂断（核心，当前通话的最终状态）
        { State::MEDIA_READY, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                // 1. 通知 Executor 发送挂断命令：ExecOutSendHangup 包裹进 ExecOutput
                core::ExecOutSendHangup execSendHangup;
                out.push_back(core::ExecOutput{execSendHangup});

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                return out;
            },
            State::LoggedIn
        },

        // ===== 新增：被动挂断（CallEnded，收到服务端通知）=====
        // 1. CALLING状态下收到被动挂断
        { State::CALLING, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                // 3. 通知 UI 通话结束：UiOutCallEnded 包裹进 UiOutput
                core::UiOutCallEnded uiCallEnded{e.peer, e.reason};
                out.push_back(core::UiOutput{uiCallEnded});

                return out;
            },
            State::LoggedIn
        },

        // 2. RINGING状态下收到被动挂断
        { State::RINGING, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                // 3. 通知 UI 通话结束：UiOutCallEnded 包裹进 UiOutput
                core::UiOutCallEnded uiCallEnded{e.peer, e.reason};
                out.push_back(core::UiOutput{uiCallEnded});

                return out;
            },
            State::LoggedIn
        },

        // 3. IN_CALL状态下收到被动挂断
        { State::IN_CALL, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                // 3. 通知 UI 通话结束：UiOutCallEnded 包裹进 UiOutput
                core::UiOutCallEnded uiCallEnded{e.peer, e.reason};
                out.push_back(core::UiOutput{uiCallEnded});

                return out;
            },
            State::LoggedIn
        },

        // 4. MEDIA_READY状态下收到被动挂断（核心）
        { State::MEDIA_READY, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);

                // 2. 状态变更：UiOutStateChanged 包裹进 UiOutput
                core::UiOutStateChanged uiStateChange{cur, State::LoggedIn};
                out.push_back(core::UiOutput{uiStateChange});

                // 3. 通知 UI 通话结束：UiOutCallEnded 包裹进 UiOutput
                core::UiOutCallEnded uiCallEnded{e.peer, e.reason};
                out.push_back(core::UiOutput{uiCallEnded});

                return out;
            },
            State::LoggedIn
        }

    };
}
