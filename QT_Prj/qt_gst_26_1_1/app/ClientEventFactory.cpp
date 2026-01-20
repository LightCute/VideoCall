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

    proto::Unknown req_error;
    req_error.message = msg;
    return ProtoEvtUnknow{req_error}; // 改为 ProtoEvt 前缀
}
