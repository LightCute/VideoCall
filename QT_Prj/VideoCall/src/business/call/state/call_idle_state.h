// src/business/connect/state/disconnected_state.h
#pragma once
#include "../../../framework/state/abstract_connect_state.h"
#include "../../../utilities/log.h"
#include "../../../framework/command/command_context.h"
#include "../../../framework/session/abstract_session.h"
#include "../../../framework/event/abstract_event.h"
#include "../event/call_event.h"
#include "../model/call_payload.h"
#include "../command/call_command.h"
#include "../receiver/call_receiver.h"

class CallIdleState : public AbstractConnectState{
public:
    void onEnter(AbstractSession* session) override {
        Log::info("[CallIdleState] Entered call idle state.");
    }

    // 仅声明，实现移到类外（确保ConnectingState完整定义可见）
    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override;

    void onExit(AbstractSession* session) override {
        Log::info("[CallIdleState] Exited call idle state.");
    }    
};

// 类外实现，此时包含ConnectingState完整定义
#include "calling_state.h"
inline std::unique_ptr<AbstractState> CallIdleState::handleEvent(
    AbstractSession* session, const AbstractEvent& event) {
    if(typeid(event) == typeid(CallEvent))
    {
        Log::debug("[CallIdleState] Received event type: [{}]", typeid(event).name());
        auto& callEvent = static_cast<const CallEvent&>(event);
        Log::info("[CallIdleState] Processing call event, peer_id: [{}]", 
                    callEvent.getPeerid());
        CommandContext ctx;
        auto now_timepoint = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
        ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
        ctx.set(CallPayload{
            callEvent.getPeerid()
        });
        auto cmd = std::make_unique<CallCommand>(ctx);
        cmd->addReceiver(std::make_shared<CallReceiver>());
        session->getDispatcher()->postCommand(std::move(cmd));

        Log::info("[CallIdleState] Transitioning to [CallingState]");
        return std::make_unique<CallingState>(); 
    }
    return nullptr;
}
