#pragma once
#include <chrono>
#include <cstdint>
#include "../../../utilities/log.h"
#include "../../../framework/state/abstract_connect_state.h"
#include "../../../framework/command/command_context.h"
#include "../../../framework/session/abstract_session.h"
#include "../command/call_built_command.h"
#include "../receiver/call_built_receiver.h"
#include "../event/call_send_event.h"
#include "../command/call_send_command.h"
#include "../receiver/call_send_receiver.h"
class CallBuiltState : public AbstractConnectState  {
public:
    void onEnter(AbstractSession* session) override {
        // 进入登录成功状态，发布状态变更事件
        Log::info("[CallBuiltState] Entered call built state.");
        CommandContext ctx;
        auto now_timepoint = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
        ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
        ctx.set(ConnectPayload{
            
        });
        auto cmd = std::make_unique<CallBuiltCommand>(ctx);
        cmd->addReceiver(std::make_shared<CallBuiltReceiver>());
        session->getDispatcher()->postCommand(std::move(cmd));
        
    }

    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override {
        Log::debug("[CallBuiltState] Received event type: [{}]", typeid(event).name());
        if(typeid(event) == typeid(CallSendEvent))
        {
            Log::debug("[CallBuiltState] Received event type: [{}]", typeid(event).name());
            auto& sendEvent = static_cast<const CallSendEvent&>(event);
            Log::info("[CallBuiltState] Processing call event, msg: [{}]", 
                         sendEvent.getMsg());
            CommandContext ctx;
            auto now_timepoint = std::chrono::system_clock::now();
            auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
            ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
            ctx.set(SendPayload{
                
                sendEvent.getMsg()
            });
            auto cmd = std::make_unique<CallSendCommand>(ctx);
            cmd->addReceiver(std::make_shared<CallSendReceiver>());
            session->getDispatcher()->postCommand(std::move(cmd));

            
        }
        return nullptr;
        
    }

    void onExit(AbstractSession* session) override {
        // 退出登录状态的清理逻辑
        Log::info("[CallBuiltState] Exited call built state.");
    }
};








