#pragma once

#include <string>
#include "protocol_types.h"

enum class ClientEventType {
    Unknown,
    LoginOk,
    LoginFail,
    OnlineUsers
};

struct ClientEvent {
    ClientEventType type = ClientEventType::Unknown;

    proto::LoginResponse loginResp;
    proto::OnlineUsers   onlineUsers;
};
