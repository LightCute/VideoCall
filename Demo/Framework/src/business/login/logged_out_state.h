#pragma once
#include "../../framework/state/abstract_login_state.h"
#include "login_event.h"
#include "../../utilities/log.h"
#include "../../framework/command/command_context.h"
#include "login_payload.h"
#include "login_command.h"
#include "login_receiver.h"
#include "../../framework/session/abstract_session.h"
#include "../../framework/event/abstract_event.h"


class LoggedOutState : public AbstractLoginState{
    void onEnter(AbstractSession* session) override {
        // 进入登录成功状态，发布状态变更事件
        Log::info("[LoggedOutState] Entered logged out state.");
    }

    std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) override {
        if(typeid(event) == typeid(LoginEvent))
        {
            Log::debug("[LoggedOutState] Received event type: [{}]", typeid(event).name());
            auto& loginEvent = static_cast<const LoginEvent&>(event);
            Log::info("[LoggedOutState] Processing login event, user: [{}], success: [{}]", 
                      loginEvent.getUserId(), loginEvent.isSuccess());
            if (loginEvent.isSuccess())
            {

                CommandContext ctx;
                //ctx.session_id = session->id();
                auto now_timepoint = std::chrono::system_clock::now();
                auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now_timepoint);
                ctx.timestamp = static_cast<uint64_t>(now_ms.time_since_epoch().count());
                ctx.set(LoginPayload{
                    loginEvent.getUserId(),
                    loginEvent.getToken()
                });
                auto cmd = std::make_unique<LoginCommand>(ctx);
                cmd->addReceiver(std::make_shared<LoginReceiver>());
                session->getDispatcher()->postCommand(std::move(cmd));
                Log::info("[LoggedOutState] Transitioning to LoggedInState");
                return std::make_unique<LoggedInState>();            
            };
        }
        return nullptr;
    }

    void onExit(AbstractSession* session) override {
        // 退出登录状态的清理逻辑
        Log::info("[LoggedOutState] Exited logged out state.");
    }    
};