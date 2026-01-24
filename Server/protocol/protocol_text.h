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
std::string makeCallIncoming(const std::string& from_user);       // 通知新来电
std::string makeCallAccepted(const std::string& peer);            // 通知通话接通
std::string makeCallRejected(const std::string& peer);            // 通知通话被拒



// parse
bool parseLoginRequest(const std::string& msg, std::string& user, std::string& pwd);
bool parseLoginResponse(const std::string& msg, LoginResponse& resp);
bool parseOnlineUsers(const std::string& msg, OnlineUsers& users);
bool parseLogout(const std::string& msg);
bool parseHeartbeat(const std::string& msg);
bool parseRegisterPeer(const std::string& msg,std::string& lan,std::string& vpn,int& port);
bool parseSendText(const std::string& msg, std::string& from_user, std::string& content);
bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content);
bool parseCallRequest(const std::string& msg, std::string& target_user); // 解析呼叫请求
bool parseCallAccept(const std::string& msg);                            // 解析接受呼叫
bool parseCallReject(const std::string& msg);                            // 解析拒绝呼叫




}
