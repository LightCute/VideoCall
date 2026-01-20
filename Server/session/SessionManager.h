//SessionManager.h
#pragma once
#include <map>
#include <mutex>
#include <string>
#include "./domain/User.h"

struct ClientNetInfo {
    std::string lanIp;
    std::string vpnIp;
    int udpPort = 0;
};

struct ClientInfo {
    domain::User user;
    bool online = false;
    std::chrono::steady_clock::time_point lastHeartbeat = std::chrono::steady_clock::now();
    ClientNetInfo net;
};

class SessionManager {
public:
    void login(int fd, const ClientInfo& info);
    void logout(int fd);
    std::map<int, ClientInfo> snapshot() const;

    void updateHeartbeat(int fd);
    void updateNetInfo(int fd, const ClientNetInfo& net);
    bool exists(int fd);
    int getFdByUsername(const std::string& username) const;
private:
    mutable std::mutex mutex_;
    std::map<int, ClientInfo> sessions_;
};
