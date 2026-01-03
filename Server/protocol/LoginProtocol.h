// LoginProtocol.h
#pragma once
#include <string>

class LoginProtocol {
public:
    enum class CommandType {
        UNKNOWN,
        LOGIN
    };

    struct LoginRequest {
        std::string username;
        std::string password;
    };

    struct LoginResponse {
        bool success;
        std::string message;
        int privilegeLevel;
    };

    // 解析客户端消息
    static CommandType parseCommand(const std::string& msg, LoginRequest& outReq);

    // 生成服务器回复
    static std::string makeLoginResponse(const LoginResponse& resp);
};
