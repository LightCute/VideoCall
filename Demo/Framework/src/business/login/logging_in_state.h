// src/business/login/logging_in_state.h
#pragma once
#include "../../framework/state/abstract_login_state.h"
#include "../../utilities/log.h"
#include "login_success_event.h"
#include "login_failed_event.h"
// 前向声明（仅声明，不定义）
class LoggedOutState;
class LoggedInState;
class AbstractSession; // 补充前向声明（用到了AbstractSession*）

class LoggingInState : public AbstractLoginState {
public:
    void onEnter(AbstractSession* session) override {
        Log::info("[LoggingInState] Entered logging in state... (waiting for network)");
    }

    // 仅声明，实现移到类外（避免循环包含）
    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override;

    void onExit(AbstractSession* session) override {
        Log::info("[LoggingInState] Exited logging in state.");
    }
};


#include "logged_out_state.h"
#include "logged_in_state.h"

inline std::unique_ptr<AbstractState> LoggingInState::handleEvent(
    AbstractSession* session, const AbstractEvent& event) {

    if (typeid(event) == typeid(LoginSuccessEvent)) {
        auto& successEvent = static_cast<const LoginSuccessEvent&>(event);
        Log::info("[LoggingInState] Login success for user: [{}]", successEvent.getUserId());
        return std::make_unique<LoggedInState>();
    }

    if (typeid(event) == typeid(LoginFailedEvent)) {
        auto& failedEvent = static_cast<const LoginFailedEvent&>(event);
        Log::warn("[LoggingInState] Login failed: [{}]", failedEvent.getErrorMsg());
        return std::make_unique<LoggedOutState>(); // 此时LoggedOutState已完整定义
    }

    return nullptr; // 其他事件返回空
}
