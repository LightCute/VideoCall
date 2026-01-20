//protocol_text.cpp
#include "protocol_text.h"
#include "protocol_common.h"

#include <sstream>

namespace proto {

/* ================= build ================= */

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


} // namespace proto
