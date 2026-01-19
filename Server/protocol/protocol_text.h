//protocol_text.h

#pragma once
#include <string>
#include "protocol_types.h"

namespace proto {

// build
std::string makeLoginRequest(const std::string& user, const std::string& pwd);
std::string makeLoginOk(const UserInfo& user, const std::string& msg);
std::string makeLoginFail(const std::string& msg);
std::string makeOnlineUsers(const OnlineUsers& users);
std::string makeLogout();
std::string makeHeartbeat();
std::string makeHeartbeatAck();   // ⭐ 新增

// parse
bool parseLoginRequest(const std::string& msg, std::string& user, std::string& pwd);
bool parseLoginResponse(const std::string& msg, LoginResponse& resp);
bool parseOnlineUsers(const std::string& msg, OnlineUsers& users);
bool parseLogout(const std::string& msg);
bool parseHeartbeat(const std::string& msg);

}
