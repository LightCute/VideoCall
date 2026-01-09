//SessionManager.h
#pragma once
#include <map>
#include <mutex>
#include <string>
#include "./domain/User.h"

struct ClientInfo {
    domain::User user;
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
