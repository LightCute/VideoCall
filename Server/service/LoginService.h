//LoginService.h
#pragma once
#include <string>

#include "./event/ServerEvent.h" // for proto::LoginRequest

using namespace proto;

namespace proto {
    struct LoginRequest;
}


struct LoginResult {
    bool success = false;
    std::string username;
    int privilege = 0;
    std::string reason;   // 失败原因
};

class LoginService {
public:
    LoginResult handleLogin(const proto::LoginRequest& req);
};
