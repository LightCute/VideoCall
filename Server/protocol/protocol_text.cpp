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
// ========== 通话协议构建函数 ==========
// 构建媒体Offer回应（服务端→客户端，携带对方IP/Port）
std::string makeMediaOfferResp(const std::string& peer, const std::string& lanIp, const std::string& vpnIp, int udpPort) {
    std::ostringstream oss;
    oss << CMD_MEDIA_OFFER_RESP << " " 
        << peer << " " 
        << lanIp << " " 
        << vpnIp << " " 
        << udpPort;
    return oss.str();
}

// 构建媒体Answer回应（服务端→客户端，携带对方IP/Port）
std::string makeMediaAnswerResp(const std::string& peer, const std::string& lanIp, const std::string& vpnIp, int udpPort) {
    std::ostringstream oss;
    oss << CMD_MEDIA_ANSWER_RESP << " " 
        << peer << " " 
        << lanIp << " " 
        << vpnIp << " " 
        << udpPort;
    return oss.str();
}

// 构建服务端通知新来电的消息（CMD_CALL_INCOMING from_user）
std::string makeCallIncoming(const std::string& from_user) {
    std::ostringstream oss;
    oss << CMD_CALL_INCOMING << " " << from_user;
    return oss.str();
}

// 构建服务端通知通话接通的消息（CMD_CALL_ACCEPTED peer）
std::string makeCallAccepted(const std::string& peer) {
    std::ostringstream oss;
    oss << CMD_CALL_ACCEPTED << " " << peer;
    return oss.str();
}

// 构建服务端通知通话被拒的消息（CMD_CALL_REJECTED peer）
std::string makeCallRejected(const std::string& peer) {
    std::ostringstream oss;
    oss << CMD_CALL_REJECTED << " " << peer;
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
    return oss.str();
}

std::string makeLogout() {
    return CMD_LOGOUT;
}

std::string makeHeartbeat() {
    return CMD_HEARTBEAT;
}

std::string makeHeartbeatAck() {
    return CMD_HEARTBEAT_ACK;  // "PONG"
}

// 构建客户端发送的文本消息（CMD_SEND_TEXT）
std::string makeSendText(const std::string& from_user, const std::string& content) {
    std::ostringstream oss;
    oss << CMD_SEND_TEXT << " " << from_user << " " << content;
    return oss.str();
}

// 构建服务端转发的文本消息（CMD_FORWARD_TEXT）
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


bool parseLogout(const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    return cmd == CMD_LOGOUT;
}

bool parseHeartbeat(const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    return cmd == CMD_HEARTBEAT;
}

bool parseRegisterPeer(
    const std::string& msg,
    std::string& lan,
    std::string& vpn,
    int& port)
{
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_REGISTER_PEER) return false;

    iss >> lan >> vpn >> port;
    return !lan.empty() && port > 0;
}

// 解析客户端发送的文本消息
bool parseSendText(const std::string& msg, std::string& from_user, std::string& content) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;

    if (cmd != CMD_SEND_TEXT) return false;
    
    iss >> from_user;
    // 读取剩余所有内容作为消息体（支持空格）
    std::getline(iss >> std::ws, content);
    return !from_user.empty() && !content.empty();
}

// 解析客户端发送的定向文本消息（格式：SEND_TEXT target_user content）
    bool parseSendTextMsg(const std::string& msg, std::string& target_user, std::string& content) {
        std::istringstream iss(msg);
        std::string cmd;
        iss >> cmd;

        if (cmd != CMD_SEND_TEXT) return false;

        // 读取目标用户名
        iss >> target_user;
        // 读取剩余所有内容作为消息（支持含空格的消息）
        std::getline(iss >> std::ws, content);
        return !target_user.empty() && !content.empty();
    }

// 解析客户端的呼叫请求（CMD_CALL target_user）
bool parseCallRequest(const std::string& msg, std::string& target_user) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    if (cmd != CMD_CALL) return false;
    iss >> target_user;
    return !target_user.empty();
}

// 解析客户端的接受呼叫请求（CMD_CALL_ACCEPT）
bool parseCallAccept(const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    return cmd == CMD_CALL_ACCEPT;
}

// 解析客户端的拒绝呼叫请求（CMD_CALL_REJECT）
bool parseCallReject(const std::string& msg) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    return cmd == CMD_CALL_REJECT;
}

// 解析客户端的媒体Offer请求
bool parseMediaOffer(const std::string& msg, std::string& target_user) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    if (cmd != CMD_MEDIA_OFFER) return false;
    iss >> target_user;
    return !target_user.empty();
}

// 解析客户端的媒体Answer请求
bool parseMediaAnswer(const std::string& msg, std::string& target_user) {
    std::istringstream iss(msg);
    std::string cmd;
    iss >> cmd;
    if (cmd != CMD_MEDIA_ANSWER) return false;
    iss >> target_user;
    return !target_user.empty();
}

} // namespace proto
