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
std::string makeCallRequest(const std::string& target_user);
std::string makeAcceptCallRequest();
std::string makeRejectCallRequest();
std::string makeMediaOfferRequest(const std::string& peer);
std::string makeMediaAnswerRequest(const std::string& peer);
// 客户端→服务端：构建主动挂断请求
std::string makeCallHangup();
// 服务端→客户端：构建挂断通知（携带原因）
std::string makeCallEnded(const std::string& peer, const std::string& reason);
// parse
bool parseLoginRequest(const std::string& msg, std::string& user, std::string& pwd);
bool parseLoginResponse(const std::string& msg, LoginResponse& resp);
bool parseOnlineUsers(const std::string& msg, OnlineUsers& users);
bool parseHeartbeatAck(const std::string& msg);
bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content);
bool parseForwardTextMsg(const std::string& msg, std::string& from_user, std::string& content);
bool parseCallIncoming(const std::string& msg, CallIncoming& out);
bool parseCallAccepted(const std::string& msg, CallAccepted& out);
bool parseCallRejected(const std::string& msg, CallRejected& out);
bool parseMediaPeerResp(const std::string& msg, MediaPeerResp& out);
// 客户端解析服务端的挂断通知
bool parseCallEnded(const std::string& msg, std::string& peer, std::string& reason);
}
