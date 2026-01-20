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
std::string makeHeartbeat();
std::string makeRegisterPeerMsg(const std::string& lan,const std::string& vpn,int port);
std::string makeSendTextMsg(const std::string& target_user, const std::string& content);
std::string makeForwardTextMsg(const std::string& from_user, const std::string& content);


// parse
bool parseLoginRequest(const std::string& msg, std::string& user, std::string& pwd);
bool parseLoginResponse(const std::string& msg, LoginResponse& resp);
bool parseOnlineUsers(const std::string& msg, OnlineUsers& users);
bool parseHeartbeatAck(const std::string& msg);
bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content);
bool parseForwardTextMsg(const std::string& msg, std::string& from_user, std::string& content);

}
