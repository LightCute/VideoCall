// src/business/connect/state/connecting_state.h
#pragma once
// 仅包含必要的抽象类（abstract_call_state.h已包含abstract_state.h，无需重复包含）
#include "../../../framework/state/abstract_call_state.h"
#include "../../../utilities/log.h"
#include "../event/call_event.h"
#include "../event/call_success_event.h"
#include "../event/call_failed_event.h"
// // 前向声明
// class AbstractSession;
// // 前向声明，避免直接包含
// class DisconnectedState;
// class ConnectedState;
class CallBuiltState; // 前向声明，替代直接包含
class CallIdleState; // 前向声明，替代直接包含
class CallingState : public AbstractCallState {
public:
    void onEnter(AbstractSession* session) override {
        Log::info("[CallingState] Entered calling state... (waiting for network)");
    }

    // 仅声明，实现移到类外
    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override;

    void onExit(AbstractSession* session) override {
        Log::info("[CallingState] Exited calling state.");
    }
};

// 类外实现，确保子类完整定义可见
// #include "disconnected_state.h"
#include "call_idle_state.h"
#include "call_built_state.h"
inline std::unique_ptr<AbstractState> CallingState::handleEvent(
    AbstractSession* session, const AbstractEvent& event) {
    if (typeid(event) == typeid(CallSuccessEvent))
    {
        Log::debug("[CallingState] Received event type: [{}]", typeid(event).name());
        Log::info("[CallingState] Processing call success event");
        Log::info("[CallingState] Transitioning to [CallBuiltState]");
        return std::make_unique<CallBuiltState>(); 
    }
    else if (typeid(event) == typeid(CallFailedEvent))
    {
        Log::debug("[CallingState] Received event type: [{}]", typeid(event).name());
        Log::info("[CallingState] Processing call failed event");
        Log::info("[CallingState] Transitioning back to [CallIdleState]");
        return std::make_unique<CallIdleState>(); 
    }
    return nullptr;
}