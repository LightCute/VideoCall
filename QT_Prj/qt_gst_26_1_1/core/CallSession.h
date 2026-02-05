// CallSession.h - 仅封装会话数据，无任何业务逻辑
#pragma once
#include <string>

// 引入旧状态枚举（若原有枚举在其他文件，直接#include即可）
enum class OldCallState {
    IDLE,        // 空闲
    CONNECTING,  // 连接中
    CONNECTED,   // 已连接
    DISCONNECTED // 已断开
};

// 纯数据载体：仅存字段，无业务方法
struct CallSession {
    // 会话唯一标识（便于后续追踪，阶段1可简单生成）
    std::string sessionId;
    // 迁移的通话资源字段
    std::string peerName;  // 对应原peer_
    std::string peerIp;    // 对应原peerIp_
    int peerPort = 0;      // 对应原peerPort_
    bool isCaller = false; // 是否为呼叫方
    // 映射旧状态，与FSM保持一致
    OldCallState callState = OldCallState::IDLE;

    // 可选：简化初始化的构造函数（非必须，提升易用性）
    CallSession() = default;
    CallSession(const std::string& id, const std::string& peer, const std::string& ip,
                int port, bool caller, OldCallState state)
        : sessionId(id), peerName(peer), peerIp(ip), peerPort(port),
        isCaller(caller), callState(state) {}
};
