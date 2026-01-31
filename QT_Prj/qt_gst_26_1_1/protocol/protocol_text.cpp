//protocol_text.cpp
#include "protocol_text.h"
#include "protocol_common.h"

#include <sstream>

namespace proto {

/* ================= build ================= */

std::string makeCallHangup() {
    return CMD_CALL_HANGUP; // 主动挂断无需额外参数，直接返回命令
}

std::string makeCallEnded(const std::string& peer, const std::string& reason) {
    std::ostringstream oss;
    oss << CMD_CALL_ENDED << " " << peer << " " << reason;
    return oss.str();
}

std::string makeCallRequest(const std::string& target_user) {
    std::ostringstream oss;
    oss << CMD_CALL << " " << target_user;
    return oss.str();
}

std::string makeAcceptCallRequest() {
    return CMD_CALL_ACCEPT;
}

std::string makeRejectCallRequest() {
    return CMD_CALL_REJECT;
}

std::string makeMediaOfferRequest(const std::string& peer) {
    std::ostringstream oss;
    oss << CMD_MEDIA_OFFER << " " << peer;
    return oss.str();
}

std::string makeMediaAnswerRequest(const std::string& peer) {
    std::ostringstream oss;
    oss << CMD_MEDIA_ANSWER << " " << peer;
    return oss.str();
}

// Client 使用
std::string makeLoginRequest(const std::string& user,
                             const std::string& pwd)
{
    std::ostringstream oss;
    oss << CMD_LOGIN << " " << user << " " << pwd ;
    return oss.str();
}

// Server 使用
std::string makeLoginOk(const UserInfo& user,
                        const std::string& msg)
{
    std::ostringstream oss;
    oss << CMD_LOGIN_OK << " "
        << user.privilege << " "
        << msg ;
    return oss.str();
}

// Server 使用
std::string makeLoginFail(const std::string& msg)
{
    std::ostringstream oss;
    oss << CMD_LOGIN_FAIL << " " << msg ;
    return oss.str();
}

// Server 使用
std::string makeOnlineUsers(const OnlineUsers& users)
{
    std::ostringstream oss;
    oss << CMD_ONLINE_USERS << " "
        << users.users.size();

    for (const auto& u : users.users) {
        oss << " " << u.username << ":" << u.privilege;
    }
    oss ;
    return oss.str();
}

std::string makeHeartbeat() {
    return CMD_HEARTBEAT;
}

std::string makeRegisterPeerMsg(
    const std::string& lan,
    const std::string& vpn,
    int port)
{
    std::ostringstream oss;
    oss << CMD_REGISTER_PEER << " "
        << lan << " "
        << vpn << " "
        << port;
    return oss.str();
}

// （客户端→服务器）
std::string makeSendTextMsg(const std::string& target_user, const std::string& content) {
    std::ostringstream oss;
    // 格式：SEND_TEXT 目标用户名 消息内容（内容含空格需保留，用引号或直接拼接，这里用空格分隔后拼接）
    oss << CMD_SEND_TEXT << " " << target_user << " " << content;
    return oss.str();
}

// （服务器→客户端）
std::string makeForwardTextMsg(const std::string& from_user, const std::string& content) {
    std::ostringstream oss;
    oss << CMD_FORWARD_TEXT << " " << from_user << " " << content;
    return oss.str();
}


/* ================= parse ================= */

bool parseCallEnded(const std::string& msg, std::string& peer, std::string& reason) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_CALL_ENDED) return false;
    iss >> peer >> reason;
    return !peer.empty() && !reason.empty();
}

// Server 使用
bool parseLoginRequest(const std::string& msg,
                       std::string& user,
                       std::string& pwd)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_LOGIN) return false;
    iss >> user >> pwd;
    return !user.empty() && !pwd.empty();
}

// Client 使用
bool parseLoginResponse(const std::string& msg,
                        LoginResponse& resp)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd == CMD_LOGIN_OK) {
        resp.success = true;
        iss >> resp.user.privilege;
        std::getline(iss, resp.message);
        if (!resp.message.empty() && resp.message[0] == ' ')
            resp.message.erase(0, 1);
        return true;
    }

    if (cmd == CMD_LOGIN_FAIL) {
        resp.success = false;
        std::getline(iss, resp.message);
        if (!resp.message.empty() && resp.message[0] == ' ')
            resp.message.erase(0, 1);
        return true;
    }

    return false;
}

// Client 使用
bool parseOnlineUsers(const std::string& msg,
                      OnlineUsers& users)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_ONLINE_USERS) return false;

    int count = 0;
    iss >> count;

    users.users.clear();

    for (int i = 0; i < count; ++i) {
        std::string token;
        iss >> token;           // user:priv
        auto pos = token.find(':');
        if (pos == std::string::npos) continue;

        UserInfo u;
        u.username = token.substr(0, pos);
        u.privilege = std::stoi(token.substr(pos + 1));
        users.users.push_back(u);
    }
    return true;
}


bool parseHeartbeatAck(const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    return cmd == CMD_HEARTBEAT_ACK;   // "PONG"
}

// 解析发送文本消息（服务器用）
bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    if (cmd != CMD_SEND_TEXT) return false;

    // 读取目标用户名
    iss >> target_user;
    // 读取剩余所有内容作为消息（处理含空格的情况）
    std::getline(iss >> std::ws, content);
    return !target_user.empty() && !content.empty();
}

// 解析转发文本消息（客户端用）
bool parseForwardTextMsg(const std::string& msg, std::string& from_user, std::string& content) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    if (cmd != CMD_FORWARD_TEXT) return false;

    // 读取发送者用户名
    iss >> from_user;
    // 读取剩余所有内容作为消息
    std::getline(iss >> std::ws, content);
    return !from_user.empty() && !content.empty();
}

// 解析来电通知（CMD_CALL_INCOMING）
bool parseCallIncoming(const std::string& msg, CallIncoming& out) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_CALL_INCOMING) return false;
    iss >> out.from;
    return !out.from.empty();
}

// 解析通话被接听（CMD_CALL_ACCEPTED）
bool parseCallAccepted(const std::string& msg, CallAccepted& out) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_CALL_ACCEPTED) return false;
    iss >> out.peer;
    return !out.peer.empty();
}

// 解析通话被拒绝（CMD_CALL_REJECTED）
bool parseCallRejected(const std::string& msg, CallRejected& out) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_CALL_REJECTED) return false;
    iss >> out.peer;
    return !out.peer.empty();
}

// 解析媒体Offer/Answer响应（CMD_MEDIA_OFFER_RESP/CMD_MEDIA_ANSWER_RESP）
bool parseMediaPeerResp(const std::string& msg, MediaPeerResp& out) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_MEDIA_OFFER_RESP && cmd != CMD_MEDIA_ANSWER_RESP) return false;
    iss >> out.peer >> out.lanIp >> out.vpnIp >> out.udpPort;

    // 基础校验：用户名和端口不能为空/无效
    return !out.peer.empty() && out.udpPort > 0;
}



} // namespace proto
