#pragma once
#include <chrono>
#include <cstdint>
#include "../../framework/state/abstract_login_state.h"
#include "login_event.h"
#include "../../utilities/log.h"
#include "../../framework/command/command_context.h"
#include "login_payload.h"
#include "login_command.h"
#include "login_receiver.h"
#include "../../framework/session/abstract_session.h"
class LoggedInState : public AbstractLoginState {
public:
    void onEnter(AbstractSession* session) override {
        // 进入登录成功状态，发布状态变更事件
        Log::info("[LoggedInState] Entered logged in state.");
    }

    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override {
        Log::debug("[LoggedInState] Received event type: [{}]", typeid(event).name());

        return nullptr;
        
    }

    void onExit(AbstractSession* session) override {
        // 退出登录状态的清理逻辑
        Log::info("[LoggedInState] Exited logged in state.");
    }
};

