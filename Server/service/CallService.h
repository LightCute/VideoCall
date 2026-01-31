// CallService.h
#pragma once
#include <map>
#include <optional>
#include <string>
#include <mutex>

// 通话会话：维护单次通话的双方和状态
struct CallSession {
    std::string caller;    // 呼叫方用户名
    std::string callee;    // 被呼叫方用户名
    
    // 新增：记录双方媒体就绪状态（核心修改）
    bool caller_media_ready = false;
    bool callee_media_ready = false;

    enum State { 
        RINGING,           // 振铃中（被呼叫方未响应）
        CONNECTED,         // 信令接通（双方同意通话，未协商媒体）
        MEDIA_NEGOTIATING, // 媒体协商中（兼容旧枚举，无实际语义）
        MEDIA_READY,        // 双方媒体都就绪
        REJECTED
    } state;
};

// 通话服务：集中管理所有活跃通话，保证线程安全
class CallService {
public:
    // 处理呼叫请求：返回创建的通话会话（失败则返回空）
    std::optional<CallSession> onCallRequest(const std::string& from, const std::string& to);
    
    // 处理接受通话：根据被呼叫方用户名找到对应的通话会话
    std::optional<CallSession> onAccept(const std::string& user);
    
    // 处理拒绝通话：同理
    std::optional<CallSession> onReject(const std::string& user);

    // 媒体协商：仅查找通话会话，不修改状态（核心修改）
    std::optional<CallSession> onMediaNegotiate(const std::string& user);

    // 标记指定用户的媒体就绪状态，返回是否双方都就绪
    bool markMediaReady(const std::string& user);    

    // 删除指定通话会话（按被呼叫方用户名）
    void deleteCallSession(const std::string& callee);

    // 通过呼叫方用户名查找通话会话（双向查找）
    std::optional<CallSession> findSessionByCaller(const std::string& caller);

    void deleteCallSessionByAnyUser(const std::string& username);
    
private:
    // 存储活跃通话：key=被呼叫方用户名（保证一个用户同时只能接一个来电）
    std::map<std::string, CallSession> activeCalls_;
    // 线程安全锁（多线程环境下保护activeCalls_）
    std::mutex mutex_;
};