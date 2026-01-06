//protocol_types.h
#pragma once
#include <string>
#include <vector>

namespace proto {

struct UserInfo {
    std::string username;
    int privilege = 0;
};

struct LoginResponse {
    bool success = false;
    UserInfo user;
    std::string message;
};

struct OnlineUsers {
    std::vector<UserInfo> users;
};

}
