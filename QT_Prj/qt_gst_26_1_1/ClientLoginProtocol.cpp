#include "ClientLoginProtocol.h"
#include <sstream>

std::string
ClientLoginProtocol::makeLoginRequest(
    const std::string& username,
    const std::string& password)
{
    std::ostringstream oss;
    oss << "LOGIN " << username << " " << password << "\n";
    return oss.str();
}

ClientLoginProtocol::ResponseType
ClientLoginProtocol::parseLoginResponse(
    const std::string& msg,
    LoginResponse& outResp)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd == "LOGIN_OK") {
        outResp.success = true;
        iss >> outResp.privilegeLevel;
        std::getline(iss, outResp.message);
        return ResponseType::LOGIN_OK;
    }

    if (cmd == "LOGIN_FAIL") {
        outResp.success = false;
        std::getline(iss, outResp.message);
        return ResponseType::LOGIN_FAIL;
    }

    return ResponseType::UNKNOWN;
}
