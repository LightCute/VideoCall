// CallService.cpp
#include "CallService.h"
#include <iostream>

// 处理呼叫请求：检查被呼叫方是否已有未处理来电
std::optional<CallSession> CallService::onCallRequest(const std::string& from, const std::string& to) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 检查被呼叫方是否已有活跃来电（防止重复呼叫）
    if (activeCalls_.find(to) != activeCalls_.end()) {
        std::cout << "[CallService] User " << to << " is already in a call" << std::endl;
        return std::nullopt;
    }
    
    // 创建新的通话会话（振铃状态）
    CallSession session{
        .caller = from,
        .callee = to,
        .state = CallSession::RINGING
    };
    activeCalls_[to] = session;
    
    std::cout << "[CallService] Create call session: " << from << " -> " << to << std::endl;
    return session;
}

// 处理接受通话：找到被呼叫方的活跃通话并更新状态
std::optional<CallSession> CallService::onAccept(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = activeCalls_.find(user);
    if (it == activeCalls_.end()) {
        std::cout << "[CallService] No incoming call for user " << user << std::endl;
        return std::nullopt;
    }
    
    // 更新通话状态为已接通
    it->second.state = CallSession::CONNECTED;
    auto session = it->second;
    
    // 接通后移除活跃通话（如需保持通话状态可保留，此处仅处理信令）
    activeCalls_.erase(it);
    
    std::cout << "[CallService] Call accepted: " << session.caller << " <-> " << session.callee << std::endl;
    return session;
}

// 处理拒绝通话：找到被呼叫方的活跃通话并更新状态
std::optional<CallSession> CallService::onReject(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = activeCalls_.find(user);
    if (it == activeCalls_.end()) {
        std::cout << "[CallService] No incoming call for user " << user << std::endl;
        return std::nullopt;
    }
    
    // 更新通话状态为已拒绝
    it->second.state = CallSession::REJECTED;
    auto session = it->second;
    
    // 拒绝后移除活跃通话
    activeCalls_.erase(it);
    
    std::cout << "[CallService] Call rejected: " << session.caller << " -> " << session.callee << std::endl;
    return session;
}

// 标记媒体协商开始
std::optional<CallSession> CallService::onMediaNegotiate(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = activeCalls_.find(user);
    if (it == activeCalls_.end()) {
        std::cout << "[CallService] No active call for media negotiate: " << user << std::endl;
        return std::nullopt;
    }
    
    // 仅处理信令已接通的会话
    if (it->second.state != CallSession::CONNECTED) {
        std::cout << "[CallService] Call not connected for media negotiate: " << user << std::endl;
        return std::nullopt;
    }
    
    it->second.state = CallSession::MEDIA_NEGOTIATING;
    std::cout << "[CallService] Media negotiate start: " << it->second.caller << " <-> " << it->second.callee << std::endl;
    return it->second;
}

// 标记媒体就绪（完成后删除会话）
void CallService::onMediaReady(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = activeCalls_.find(user);
    if (it != activeCalls_.end()) {
        it->second.state = CallSession::MEDIA_READY;
        std::cout << "[CallService] Media ready, remove session: " << it->second.caller << " <-> " << it->second.callee << std::endl;
        activeCalls_.erase(it);
    }
}



