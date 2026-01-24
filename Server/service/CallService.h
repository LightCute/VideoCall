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
    enum State { 
        RINGING,   // 振铃中（被呼叫方未响应）
        CONNECTED, // 通话已接通
        REJECTED   // 通话被拒绝
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

private:
    // 存储活跃通话：key=被呼叫方用户名（保证一个用户同时只能接一个来电）
    std::map<std::string, CallSession> activeCalls_;
    // 线程安全锁（多线程环境下保护activeCalls_）
    std::mutex mutex_;
};