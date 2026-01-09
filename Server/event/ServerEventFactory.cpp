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

    // ❗ 如果你愿意，可以抛异常 / optional
    // fallback -> 返回 ErrorEvent
    return event::ErrorEvent{
        .rawMsg = msg,
        .reason = "Unknown or malformed command"
    };
}
