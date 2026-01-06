//SessionManager.h
#pragma once
#include <map>
#include <mutex>
#include <string>

struct ClientInfo {
    std::string username;
    int privilege = 0;
    bool online = false;
};

class SessionManager {
public:
    void login(int fd, const ClientInfo& info);
    void logout(int fd);
    std::map<int, ClientInfo> snapshot() const;

private:
    mutable std::mutex mutex_;
    std::map<int, ClientInfo> sessions_;
};
