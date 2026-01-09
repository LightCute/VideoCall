//LoginService.h
#pragma once
#include <string>

#include "./event/ServerEvent.h" // for proto::LoginRequest





struct LoginResult {
    bool success = false;
    std::string username;
    int privilege = 0;
    std::string reason;   // 失败原因
    //UserInfo user;
};

class LoginService {
public:
    LoginResult handleLogin(const event::LoginRequest& req);
};
