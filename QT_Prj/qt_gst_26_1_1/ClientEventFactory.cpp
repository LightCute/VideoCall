#include "ClientEventFactory.h"

// 只在 cpp 中 include 解析函数
#include "protocol_text.h"
// 例如：
// bool parseLoginResponse(const std::string&, LoginResponse&)
// bool parseOnlineUsers(const std::string&, OnlineUsers&)

ClientEvent ClientEventFactory::makeEvent(const std::string& msg)
{
    ClientEvent event;

    // 1️⃣ Login 响应
    if (proto::parseLoginResponse(msg, event.loginResp)) {
        event.type = event.loginResp.success
                         ? ClientEventType::LoginOk
                         : ClientEventType::LoginFail;
        return event;
    }

    // 2️⃣ 在线用户列表
    if (proto::parseOnlineUsers(msg, event.onlineUsers)) {
        event.type = ClientEventType::OnlineUsers;
        return event;
    }

    // 3️⃣ 未识别
    event.type = ClientEventType::Unknown;
    return event;
}
