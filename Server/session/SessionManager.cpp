//SessionManager.cpp
#include "./session/SessionManager.h"
#include "iostream"
void SessionManager::login(int fd, const ClientInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[fd] = info;
    std::cout << "[SessionManager] User logged in: FD=" << fd 
              << ", Username=" << info.user.username << std::endl;
}

void SessionManager::logout(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(fd);
}

std::map<int, ClientInfo> SessionManager::snapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_;
}

void SessionManager::updateHeartbeat(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(fd);
    if (it != sessions_.end()) {
        it->second.lastHeartbeat = std::chrono::steady_clock::now();
        std::cout << "[SessionManager] Updated heartbeat: FD=" << fd << std::endl;
    }
    else {
         std::cout << "[SessionManager] Failed to update heartbeat: FD=" << fd << " does not exist" << std::endl;
    }
}

void SessionManager::updateNetInfo(int fd, const ClientNetInfo& net) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(fd);
    if (it != sessions_.end()) {
        it->second.net = net;
        std::cout << "[SessionManager] Updated net info for fd=" << fd
                  << " lan=" << net.lanIp
                  << " vpn=" << net.vpnIp
                  << " port=" << net.udpPort << std::endl;
    }
}

bool SessionManager::exists(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.count(fd) > 0;
}

