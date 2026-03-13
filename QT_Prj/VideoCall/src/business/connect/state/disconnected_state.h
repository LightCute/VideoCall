// src/business/connect/state/disconnected_state.h
#pragma once
#include "../../../framework/state/abstract_connect_state.h"
#include "../../../utilities/log.h"
#include "../../../framework/command/command_context.h"
#include "../../../framework/session/abstract_session.h"
#include "../../../framework/event/abstract_event.h"
#include "../event/connect_server_event.h"
#include "../model/connect_payload.h"
#include "../command/connect_command.h"
#include "../receiver/connect_receiver.h"

// 前向声明，打破循环包含
class ConnectingState;

class DisconnectedState : public AbstractConnectState{
public:
    void onEnter(AbstractSession* session) override {
        Log::info("[DisconnectedState] Entered disconnected state.");
    }

    // 仅声明，实现移到类外（确保ConnectingState完整定义可见）
    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override;

    void onExit(AbstractSession* session) override {
        Log::info("[DisconnectedState] Exited disconnected state.");
    }    
};

// 类外实现，此时包含ConnectingState完整定义
#include "connecting_state.h"
inline std::unique_ptr<AbstractState> DisconnectedState::handleEvent(
    AbstractSession* session, const AbstractEvent& event) {
    if(typeid(event) == typeid(ConnectServerEvent))
    {
        Log::debug("[DisconnectedState] Received event type: [{}]", typeid(event).name());
        auto& connectEvent = static_cast<const ConnectServerEvent&>(event);
        Log::info("[DisconnectedState] Processing connect event, url: [{}]", 
                    connectEvent.getUrl());
        CommandContext ctx;
        auto now_timepoint = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
        ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
        ctx.set(ConnectPayload{
            connectEvent.getUrl()
        });
        auto cmd = std::make_unique<ConnectCommand>(ctx);
        cmd->addReceiver(std::make_shared<ConnectReceiver>());
        session->getDispatcher()->postCommand(std::move(cmd));
        Log::info("[DisconnectedState] Transitioning to [ConnectingState]");
        return std::make_unique<ConnectingState>(); 
    }
    return nullptr;
}