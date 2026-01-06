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
