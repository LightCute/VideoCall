//SessionManager.cpp
#include "./session/SessionManager.h"

void SessionManager::login(int fd, const ClientInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[fd] = info;
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
    }
}
