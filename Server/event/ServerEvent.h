//ServerEvent.h
#pragma once
#include <string>

namespace proto {
    struct LoginRequest {
        std::string username;
        std::string password;
    };
}

enum class ServerEventType {
    LoginRequest,
    Unknown
};

struct ServerEvent {
    ServerEventType type = ServerEventType::Unknown;

    // payload（只用一个）
    proto::LoginRequest loginReq;
};
