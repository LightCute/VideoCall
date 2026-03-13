// src/business/connect/state/connecting_state.h
#pragma once
// 仅包含必要的抽象类（abstract_call_state.h已包含abstract_state.h，无需重复包含）
#include "../../../framework/state/abstract_call_state.h"
#include "../../../utilities/log.h"
#include "../event/call_event.h"

// // 前向声明
// class AbstractSession;
// // 前向声明，避免直接包含
// class DisconnectedState;
// class ConnectedState;

class CallingState : public AbstractCallState {
public:
    void onEnter(AbstractSession* session) override {
        Log::info("[CallingState] Entered calling state... (waiting for network)");
    }

    // 仅声明，实现移到类外
    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override{}

    void onExit(AbstractSession* session) override {
        Log::info("[CallingState] Exited calling state.");
    }
};

// 类外实现，确保子类完整定义可见
// #include "disconnected_state.h"
// #include "connected_state.h"
// inline std::unique_ptr<AbstractState> ConnectingState::handleEvent(
//     AbstractSession* session, const AbstractEvent& event) {
//     if (typeid(event) == typeid(ConnectSuccessEvent))
//     {
//         Log::debug("[ConnectingState] Received event type: [{}]", typeid(event).name());
//         Log::info("[ConnectingState] Processing connect success event");
//         Log::info("[ConnectingState] Transitioning to [ConnectedState]");
//         return std::make_unique<ConnectedState>(); 
//     }
//     else if (typeid(event) == typeid(ConnectFailedEvent))
//     {
//         Log::debug("[ConnectingState] Received event type: [{}]", typeid(event).name());
//         Log::info("[ConnectingState] Processing connect failed event");
//         Log::info("[ConnectingState] Transitioning back to [DisconnectedState]");
//         return std::make_unique<DisconnectedState>(); 
//     }
//     return nullptr;
// }