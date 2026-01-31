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
        .caller_media_ready = false,
        .callee_media_ready = false,
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
    
    // 更新通话状态为已接通（仅更新状态，不删除会话）
    it->second.state = CallSession::CONNECTED;
    auto session = it->second;
    
    std::cout << "[CallService] Call accepted: " << session.caller << " <-> " << session.callee << std::endl;
    return session; // ✅ 保留会话，供后续媒体协商使用
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
    
    // 拒绝后移除活跃通话（仅拒绝场景删除，媒体就绪不删除）
    activeCalls_.erase(it);
    
    std::cout << "[CallService] Call rejected: " << session.caller << " -> " << session.callee << std::endl;
    return session;
}

// 媒体协商：仅查找通话会话，不修改状态（核心修改）
std::optional<CallSession> CallService::onMediaNegotiate(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);

    // 双向查找：既找作为callee的情况，也找作为caller的情况
    for (auto& [_, session] : activeCalls_) {
        if (session.caller == user || session.callee == user) {
            // 仅返回会话，不修改状态（删除原状态修改逻辑）
            return session;
        }
    }
    return std::nullopt;
}

// 新增：标记用户媒体就绪，返回是否双方都就绪（核心实现）
bool CallService::markMediaReady(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& [key, session] : activeCalls_) {
        // 标记当前用户的媒体就绪状态
        if (session.caller == user) {
            session.caller_media_ready = true;
        } else if (session.callee == user) {
            session.callee_media_ready = true;
        } else {
            continue;
        }

        // 检查是否双方都就绪
        if (session.caller_media_ready && session.callee_media_ready) {
            session.state = CallSession::MEDIA_READY;
            // 双方就绪后可选择保留会话（如需支持挂断），也可删除
            // activeCalls_.erase(key); // 可选：双方就绪后删除会话
            return true;
        }
        return false; // 仅单方就绪
    }
    return false; // 未找到通话会话
}


void CallService::deleteCallSession(const std::string& callee) {
    std::lock_guard<std::mutex> lock(mutex_);
    activeCalls_.erase(callee);
    std::cout << "[CallService] Delete call session for callee: " << callee << std::endl;
}

std::optional<CallSession> CallService::findSessionByCaller(const std::string& caller) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [callee, session] : activeCalls_) {
        if (session.caller == caller) {
            return session;
        }
    }
    return std::nullopt;
}

// 按任意一方用户名删除会话（兼容caller/callee，通用清理）
void CallService::deleteCallSessionByAnyUser(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = activeCalls_.begin(); it != activeCalls_.end(); ) {
        const auto& session = it->second;
        if (session.caller == username || session.callee == username) {
            std::cout << "[CallService] Delete call session (any user): " 
                      << session.caller << " <-> " << session.callee << std::endl;
            it = activeCalls_.erase(it);
        } else {
            ++it;
        }
    }
}
