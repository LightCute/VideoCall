// core/FSM.cpp
#include "FSM.h"
#include <set>

FSM::FSM() {
    initTable();
}

bool FSM::isOnlineState(State s) {
    return s != State::Disconnected;
}

// 补充 core:: 前缀
std::vector<core::CoreOutput>
FSM::handle(State current, const core::CoreInput& ev)
{
    EventType evType = eventTypeFromInput(ev);
    std::cout << "[FSM] Current state: " << stateToString(current)
              << ", Received event type: " << EventTypeToString(evType) << std::endl;
    if (isOnlineState(current) && evType == EventType::HeartbeatTick) {
        // 直接返回，不走 FSM 表
        std::cout << "[FSM] Online state, directly handle HeartbeatTick event" << std::endl;
        return { core::OutSendPing{} };
    }

    else if (isOnlineState(current) && evType == EventType::HeartbeatOk) {
        // 直接返回，不走 FSM 表
        return { core::OutUpdateAlive{} };
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
            return {
                core::OutStateChanged{current, State::Disconnected},
                core::OutDisconnected{}
            };
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
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                  std::vector<core::CoreOutput> out;
                  // 替换为 InCmdConnect
                  if (auto e = std::get_if<core::InCmdConnect>(&ev)) {
                      out.push_back(core::OutStateChanged{cur, State::Connecting}); // 补充 core::
                      out.push_back(core::OutConnect{e->host, e->port}); // 补充 core::
                  }
                  return out;
              },
              State::Connecting
          },

          // ===== 正在连接 =====
          { State::Connecting, EventType::TcpConnected,
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                  std::vector<core::CoreOutput> out;
                  // 替换为 InTcpConnected
                  if (auto e = std::get_if<core::InTcpConnected>(&ev)) {
                      out.push_back(core::OutStateChanged{cur, State::Connected}); // 补充 core::
                  }
                  return out;
              },
              State::Connected
          },

          { State::Connecting, EventType::TcpDisconnected,
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput&){ return std::vector<core::CoreOutput>{}; },
              State::Disconnected
          },

          // ===== 已连接 =====
          { State::Connected, EventType::CmdLogin,
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                  std::vector<core::CoreOutput> out;
                  // 替换为 InCmdLogin
                  if (auto e = std::get_if<core::InCmdLogin>(&ev)) {
                      out.push_back(core::OutStateChanged{cur, State::LoggingIn}); // 补充 core::
                      out.push_back(core::OutSendLogin{e->user, e->pass}); // 补充 core::
                  }
                  return out;
              },
              State::LoggingIn
          },

          // ===== 正在登录 =====
          { State::LoggingIn, EventType::LoginOk ,
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                  std::vector<core::CoreOutput> out;
                  // 替换为 InLoginOk
                  if (auto e = std::get_if<core::InLoginOk>(&ev)) {
                      out.push_back(core::OutLoginOk{}); // 补充 core::
                      out.push_back(core::OutStateChanged{cur, State::LoggedIn}); // 补充 core::
                  }
                  return out;
              },
              State::LoggedIn
          },

          { State::LoggingIn, EventType::LoginFail ,
              // 补充 core:: 前缀
              [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                  std::vector<core::CoreOutput> out;
                  // 替换为 InLoginFail
                  if (auto e = std::get_if<core::InLoginFail>(&ev)) {
                      out.push_back(core::OutLoginFail{e->msg}); // 补充 core::
                      out.push_back(core::OutStateChanged{cur, State::Connected}); // 补充 core::
                  }
                  return out;
              },
              State::Connected
          },


        { State::LoggedIn, EventType::OnlineUsers,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;

                if (auto e = std::get_if<core::InOnlineUsers>(&ev)) {
                    out.push_back(core::OutOnlineUsers{e->users});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::SelectLan,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;

                if (auto e = std::get_if<core::InSelectLan>(&ev)) {
                    out.push_back(core::OutSelectLan{});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::SelectVpn,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;

                if (auto e = std::get_if<core::InSelectVpn>(&ev)) {
                    out.push_back(core::OutSelectVpn{});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::CmdSendText,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCmdSendText>(&ev)) {
                    out.push_back(core::OutSendText{e->target_user, e->content});
                }
                return out;
            },
            State::LoggedIn
        },

        { State::LoggedIn, EventType::ForwardText,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InForwardText>(&ev)) {
                    out.push_back(core::OutForwardText{e->from_user, e->content});
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
                    out.push_back(core::OutStateChanged{cur, State::CALLING});
                    out.push_back(core::OutSendCall{e->target_user});
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
                    out.push_back(core::OutStateChanged{cur, State::RINGING});
                    out.push_back(core::OutShowIncomingCall{e->from});
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
                 out.push_back(core::OutSendAcceptCall{});
                 //out.push_back(core::OutSendMediaOffer{e->peer});
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
                    out.push_back(core::OutSendRejectCall{});
                    out.push_back(core::OutStateChanged{cur, State::LoggedIn});
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
                    out.push_back(core::OutStateChanged{cur, State::IN_CALL});
                    out.push_back(core::OutSendMediaOffer{e->peer});
                }
                return out;
            },
            State::IN_CALL
        },

        { State::CALLING, EventType::MediaPeer,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InMediaPeer>(&ev)) {
                    //std::string peerIp = !e->vpnIp.empty() ? e->vpnIp : e->lanIp;
                    std::string peerIp = e->lanIp;
                    out.push_back(core::OutStateChanged{cur, State::MEDIA_READY});
                    out.push_back(core::OutMediaReady{e->lanIp, e->vpnIp, e->udpPort});
                    // CALLING 状态下收到 MediaPeer，说明是被动方收到 OFFER_RESP，需要发送 ANSWER
                    out.push_back(core::OutSendMediaAnswer{e->peer});
                }
                return out;
            },
            State::MEDIA_READY
        },

        { State::RINGING, EventType::CallAccepted,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InCallAccepted>(&ev)) {
                    out.push_back(core::OutStateChanged{cur, State::IN_CALL});
                     out.push_back(core::OutSendMediaOffer{e->peer});
                }
                return out;
            },
            State::IN_CALL
        },

        // 6. CALLING状态下收到通话被拒绝
        { State::CALLING, EventType::CallRejected,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                return out;
            },
            State::LoggedIn
        },

        // 7. IN_CALL状态下收到媒体信息
        { State::IN_CALL, EventType::MediaPeer,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                if (auto e = std::get_if<core::InMediaPeer>(&ev)) {
                    //std::string peerIp = !e->vpnIp.empty() ? e->vpnIp : e->lanIp;
                    std::string peerIp = e->lanIp;
                    out.push_back(core::OutStateChanged{cur, State::MEDIA_READY});
                    out.push_back(core::OutMediaReady{e->lanIp, e->vpnIp, e->udpPort});

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
                out.push_back(core::OutSendHangup{});          // 上报服务端
                out.push_back(core::OutStopMedia{});           // 停止本地媒体
                out.push_back(core::OutStateChanged{cur, State::LoggedIn}); // 回到登录状态
                out.push_back(core::OutCallEnded{"", "local_hangup"}); // 通知UI
                return out;
            },
            State::LoggedIn
        },

        // 2. RINGING状态下主动挂断
        { State::RINGING, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                out.push_back(core::OutSendHangup{});
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{"", "local_hangup"});
                return out;
            },
            State::LoggedIn
        },

        // 3. IN_CALL状态下主动挂断
        { State::IN_CALL, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                out.push_back(core::OutSendHangup{});
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{"", "local_hangup"});
                return out;
            },
            State::LoggedIn
        },

        // 4. MEDIA_READY状态下主动挂断（核心，当前通话的最终状态）
        { State::MEDIA_READY, EventType::CmdHangup,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                out.push_back(core::OutSendHangup{});
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{"", "local_hangup"});
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
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{e.peer, e.reason});
                return out;
            },
            State::LoggedIn
        },

        // 2. RINGING状态下收到被动挂断
        { State::RINGING, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{e.peer, e.reason});
                return out;
            },
            State::LoggedIn
        },

        // 3. IN_CALL状态下收到被动挂断
        { State::IN_CALL, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{e.peer, e.reason});
                return out;
            },
            State::LoggedIn
        },

        // 4. MEDIA_READY状态下收到被动挂断（核心）
        { State::MEDIA_READY, EventType::CallEnded,
            [](State cur, const core::CoreInput& ev) -> std::vector<core::CoreOutput> {
                std::vector<core::CoreOutput> out;
                auto& e = std::get<core::InCallEnded>(ev);
                out.push_back(core::OutStopMedia{});
                out.push_back(core::OutStateChanged{cur, State::LoggedIn});
                out.push_back(core::OutCallEnded{e.peer, e.reason});
                return out;
            },
            State::LoggedIn
        }

    };
}
