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
    oss ;
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

} // namespace proto
