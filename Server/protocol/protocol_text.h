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
std::string makeSendText(const std::string& from_user, const std::string& content);
std::string makeForwardTextMsg(const std::string& from_user, const std::string& content);

// parse
bool parseLoginRequest(const std::string& msg, std::string& user, std::string& pwd);
bool parseLoginResponse(const std::string& msg, LoginResponse& resp);
bool parseOnlineUsers(const std::string& msg, OnlineUsers& users);
bool parseLogout(const std::string& msg);
bool parseHeartbeat(const std::string& msg);
bool parseRegisterPeer(const std::string& msg,std::string& lan,std::string& vpn,int& port);
bool parseSendText(const std::string& msg, std::string& from_user, std::string& content);
bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content);

}
