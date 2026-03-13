#pragma once
#include <chrono>
#include <cstdint>
#include "../../../utilities/log.h"
#include "../../../framework/state/abstract_connect_state.h"
#include "../../../framework/command/command_context.h"
#include "../../../framework/session/abstract_session.h"
#include "../command/connected_command.h"
#include "../receiver/connected_receiver.h"
class ConnectedState : public AbstractConnectState  {
public:
    void onEnter(AbstractSession* session) override {
        // 进入登录成功状态，发布状态变更事件
        Log::info("[ConnectedState] Entered connected state.");
        CommandContext ctx;
        auto now_timepoint = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
        ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
        ctx.set(ConnectPayload{
            
        });
        auto cmd = std::make_unique<ConnectedCommand>(ctx);
        cmd->addReceiver(std::make_shared<ConnectedReceiver>());
        session->getDispatcher()->postCommand(std::move(cmd));
        
    }

    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override {
        Log::debug("[ConnectedState] Received event type: [{}]", typeid(event).name());

        return nullptr;
        
    }

    void onExit(AbstractSession* session) override {
        // 退出登录状态的清理逻辑
        Log::info("[ConnectedState] Exited connected state.");
    }
};








