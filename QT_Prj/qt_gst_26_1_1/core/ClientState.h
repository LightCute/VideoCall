// ClientState.h
#pragma once

#include <string>

enum class State {
    Disconnected,
    Connecting,
    Connected,
    LoggingIn,
    LoggedIn,
    CALLING,    // 发起呼叫中
    RINGING,    // 收到来电响铃中
    IN_CALL,    // 信令接通
    MEDIA_READY // 媒体信息就绪
};

// 将 State 转换为字符串
inline std::string stateToString(State s) {
    switch (s) {
    case State::Disconnected: return "Disconnected";
    case State::Connecting:   return "Connecting";
    case State::Connected:    return "Connected";
    case State::LoggingIn:    return "LoggingIn";
    case State::LoggedIn:     return "LoggedIn";
    case State::CALLING:    return "CALLING";
    case State::RINGING:    return "RINGING";
    case State::IN_CALL:    return "IN_CALL";
    case State::MEDIA_READY:return "MEDIA_READY";
    default:                  return "Unknown";
    }
}
