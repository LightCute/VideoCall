//SessionManager.h
#pragma once
#include <map>
#include <mutex>
#include <string>
#include "./domain/User.h"

struct ClientInfo {
    domain::User user;
    bool online = false;
    std::chrono::steady_clock::time_point lastHeartbeat = std::chrono::steady_clock::now();
};

class SessionManager {
public:
    void login(int fd, const ClientInfo& info);
    void logout(int fd);
    std::map<int, ClientInfo> snapshot() const;

    void updateHeartbeat(int fd);
private:
    mutable std::mutex mutex_;
    std::map<int, ClientInfo> sessions_;
};
