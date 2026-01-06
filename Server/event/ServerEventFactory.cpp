//ServerEventFactory.cpp
#include "./event/ServerEventFactory.h"
#include "protocol/protocol_text.h"

ServerEvent ServerEventFactory::makeEvent(const std::string& msg)
{
    ServerEvent e;

    if (proto::parseLoginRequest(
            msg,
            e.loginReq.username,
            e.loginReq.password))
    {
        e.type = ServerEventType::LoginRequest;
        return e;
    }

    e.type = ServerEventType::Unknown;
    return e;
}
