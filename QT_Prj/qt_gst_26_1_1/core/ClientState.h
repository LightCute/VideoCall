// ClientState.h
#pragma once

#include <string>

enum class State {
    Disconnected,
    Connecting,
    Connected,
    LoggingIn,
    LoggedIn
};

// 将 State 转换为字符串
inline std::string stateToString(State s) {
    switch (s) {
    case State::Disconnected: return "Disconnected";
    case State::Connecting:   return "Connecting";
    case State::Connected:    return "Connected";
    case State::LoggingIn:    return "LoggingIn";
    case State::LoggedIn:     return "LoggedIn";
    default:                  return "Unknown";
    }
}
