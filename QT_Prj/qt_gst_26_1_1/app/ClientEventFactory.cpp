//ClientEventFactory.cpp
#include "ClientEventFactory.h"

// 只在 cpp 中 include 解析函数
#include "protocol_text.h"

ClientEventFactory::ClientEventFactory() {}

ClientEvent ClientEventFactory::makeEvent(const std::string& msg)
{
    proto::LoginResponse login;
    if (proto::parseLoginResponse(msg, login)) {
        if (login.success)
            return ProtoEvtLoginOk{login}; // 改为 ProtoEvt 前缀
        else
            return ProtoEvtLoginFail{login}; // 改为 ProtoEvt 前缀
    }

    proto::OnlineUsers users;
    if (proto::parseOnlineUsers(msg, users)) {
        return ProtoEvtOnlineUsers{users}; // 改为 ProtoEvt 前缀
    }

    if (proto::parseHeartbeatAck(msg)) {
        return ProtoEvHeartbeatAck{}; // 改为 ProtoEvt 前缀
    }

    // 解析转发文本消息（CMD_FORWARD_TEXT）
    std::string from_user, content;
    if (proto::parseForwardTextMsg(msg, from_user, content)) {
        return ProtoEvtForwardText{from_user, content};
    }

    // =====调用通话相关解析函数 =====
    proto::CallIncoming callIn;
    if (proto::parseCallIncoming(msg, callIn)) {
        return ProtoEvtCallIncoming{callIn.from};
    }

    proto::CallAccepted callAc;
    if (proto::parseCallAccepted(msg, callAc)) {
        return ProtoEvtCallAccepted{callAc.peer};
    }

    proto::CallRejected callRe;
    if (proto::parseCallRejected(msg, callRe)) {
        return ProtoEvtCallRejected{callRe.peer};
    }

    proto::MediaPeerResp mediaPeer;
    if (proto::parseMediaPeerResp(msg, mediaPeer)) {
        return ProtoEvtMediaPeer{
            mediaPeer.peer,
            mediaPeer.lanIp,
            mediaPeer.vpnIp,
            mediaPeer.udpPort
        };
    }

    // 解析CALL_ENDED通知
    std::string peer, reason;
    if (proto::parseCallEnded(msg, peer, reason)) {
        return ProtoEvtCallEnded{peer, reason};
    }

    proto::Unknown req_error;
    req_error.message = msg;
    return ProtoEvtUnknow{req_error}; // 改为 ProtoEvt 前缀
}
