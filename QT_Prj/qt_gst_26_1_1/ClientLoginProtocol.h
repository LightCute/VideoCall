#pragma once
#include <string>

class ClientLoginProtocol {
public:
    enum class ResponseType {
        UNKNOWN,
        LOGIN_OK,
        LOGIN_FAIL,
        ONLINE_USERS
    };

    struct LoginResponse {
        bool success = false;
        int privilegeLevel = 0;
        std::string message;
    };

    // 生成登录请求
    static std::string makeLoginRequest(
        const std::string& username,
        const std::string& password
        );

    // 解析服务器返回
    static ResponseType parseLoginResponse(
        const std::string& msg,
        LoginResponse& outResp
        );
};
