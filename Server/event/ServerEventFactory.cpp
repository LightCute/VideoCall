// ServerEventFactory.cpp
#include "ServerEventFactory.h"
#include "protocol/protocol_text.h"
#include <stdexcept>


ServerEvent ServerEventFactory::makeEvent(const std::string& msg)
{
    std::string user, pwd;
    if (proto::parseLoginRequest(msg, user, pwd)) {
        return event::LoginRequest{
            .username = user,
            .password = pwd
        };
    }

    if (proto::parseLogout(msg)) {
        return event::Logout{};
    }    

    if (proto::parseHeartbeat(msg)) {
        return event::Heartbeat{};
    }

    std::string lan, vpn;
    int port;
    if (proto::parseRegisterPeer(msg, lan, vpn, port)) {
        return event::RegisterPeer{
            .lanIp = lan,
            .vpnIp = vpn,
            .udpPort = port
        };
    }

    std::string target_user, content;
    if (proto::parseSendTextMsg(msg, target_user, content)) {
        return event::SendTextToUser{
            .target_user = target_user,
            .content = content,
            .from_user = "" // 发送者用户名由 Dispatcher 补全
        };
    }
        
    std::string call_target;
    if (proto::parseCallRequest(msg, call_target)) {
        return event::CallRequest{.target_user = call_target};
    }

    if (proto::parseCallAccept(msg)) {
        return event::CallAccept{};
    }

    if (proto::parseCallReject(msg)) {
        return event::CallReject{};
    }

    // 解析媒体Offer请求
    std::string media_target;
    if (proto::parseMediaOffer(msg, media_target)) {
        return event::MediaOffer{.target_user = media_target};
    }

    // 解析媒体Answer请求
    if (proto::parseMediaAnswer(msg, media_target)) {
        return event::MediaAnswer{.target_user = media_target};
    }

    if (proto::parseCallHangup(msg)) {
    return event::CallHangup{};
    }

    // 新增：完善 parseCallEnded 逻辑（核心修改，用上parseCallEnded并传递信息）
    std::string peer;
    std::string reason;
    if (proto::parseCallEnded(msg, peer, reason)) {
        // 将解析出的peer和reason封装为CallEnded事件返回，传递到下一层级
        return event::CallEnded{
            .peer = peer,
            .reason = reason
        };
    }    

    // ❗ 如果你愿意，可以抛异常 / optional
    // fallback -> 返回 ErrorEvent
    return event::ErrorEvent{
        .rawMsg = msg,
        .reason = "Unknown or malformed command"
    };
}
