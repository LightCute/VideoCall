//ClientEventFactory.cpp
#include "ClientEventFactory.h"

// 只在 cpp 中 include 解析函数
#include "protocol_text.h"
// 例如：
// bool parseLoginResponse(const std::string&, LoginResponse&)
// bool parseOnlineUsers(const std::string&, OnlineUsers&)

ClientEvent ClientEventFactory::makeEvent(const std::string& msg)
{
    proto::LoginResponse login;
    if (proto::parseLoginResponse(msg, login)) {
        if (login.success)
            return EvLoginOk{login};
        else
            return EvLoginFail{login};
    }

    proto::OnlineUsers users;
    if (proto::parseOnlineUsers(msg, users)) {
        return EvOnlineUsers{users};
    }

    proto::Unknown req_error;
    req_error.message = msg;
    return EvUnknow{req_error};
}
