#include "LoginProtocol.h"
#include <sstream>

LoginProtocol::CommandType
LoginProtocol::parseCommand(const std::string& msg, LoginRequest& outReq)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd == "LOGIN") {
        iss >> outReq.username >> outReq.password;
        return CommandType::LOGIN;
    }

    return CommandType::UNKNOWN;
}

std::string
LoginProtocol::makeLoginResponse(const LoginResponse& resp)
{
    std::ostringstream oss;

    if (resp.success) {
        oss << "LOGIN_OK "
            << resp.privilegeLevel << " "
            << resp.message << "\n";
    } else {
        oss << "LOGIN_FAIL "
            << resp.message << "\n";
    }

    return oss.str();
}
