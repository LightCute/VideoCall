// core/CallSession.h
#pragma once
#include <string>
#include <cstdint>
#include "ClientState.h" // 引入旧 State 用于映射

// 最小版 CallState：仅映射旧 State 中的通话状态，不新增 Ending/MediaReady
enum class CallState {
    Idle,        // 对应旧 State::LoggedIn（无通话）
    Calling,     // 对应旧 State::CALLING
    Ringing,     // 对应旧 State::RINGING
    InCall,      // 对应旧 State::IN_CALL
    MediaReady,  // 对应旧 State::MEDIA_READY
    Ending       // 新增：标记会话正在结束，防重复调用
};

// 生成唯一 SessionId（极简版：自增整数，线程安全）
static std::atomic<uint64_t> g_next_session_id = 1;
inline uint64_t generateUniqueSessionId() {
    return g_next_session_id.fetch_add(1);
}

// 最小版 CallSession：仅包含基础字段，无任何业务逻辑
struct CallSession {
    uint64_t sessionId;          // 唯一标识
    std::string peerName;        // 对应旧 peer_
    std::string peerIp;          // 对应旧 peerIp_
    int peerPort = 0;            // 对应旧 peerPort_
    bool isCaller = false;       // 对应旧 isCaller_
    CallState currentCallState;  // 映射旧 State 的通话状态

    // 构造函数：初始化基础字段
    CallSession() : sessionId(generateUniqueSessionId()), currentCallState(CallState::Idle) {}

    // 辅助：CallState 转旧 State（用于兼容，不修改旧逻辑）
    State toOldState() const {
        switch (currentCallState) {
        case CallState::Calling: return State::CALLING;
        case CallState::Ringing: return State::RINGING;
        case CallState::InCall: return State::IN_CALL;
        case CallState::MediaReady: return State::MEDIA_READY;
        case CallState::Ending: return State::LoggedIn; // 新增
        default: return State::LoggedIn;
        }
    }
};

// 辅助：旧 State 转 CallState（用于初始化 Session）
inline CallState fromOldState(State oldState) {
    switch (oldState) {
    case State::CALLING: return CallState::Calling;
    case State::RINGING: return CallState::Ringing;
    case State::IN_CALL: return CallState::InCall;
    case State::MEDIA_READY: return CallState::MediaReady;
    default: return CallState::Idle;
    }
}
