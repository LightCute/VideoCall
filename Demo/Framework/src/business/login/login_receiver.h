#pragma once
#include <thread>
#include "../../framework/command/abstract_receiver.h"
#include "../../framework/command/command_context.h"
#include "login_payload.h"
#include "../../utilities/log.h"
#include "../../service/abstract_ui.h"
#include "../../service/abstract_net.h"
#include "../../application/app_context.h"
#include "../../framework/event/event_bus.h"
#include "login_success_event.h"
#include "login_failed_event.h"

class LoginReceiver : public AbstractReceiver {
public:
    void performAction(const CommandContext& context) override {
        auto& login = context.get<LoginPayload>();
        Log::info("[LoginReceiver] Processing login - user = [{}], token = [{}]", 
                  login.user, login.token);

        auto* ui = AppContext::instance().ui;
        auto* net = AppContext::instance().net;

        if(ui) ui->showMessage("[System] Logging in...");
        if(net) {
            net->connect("server.example.com");
            net->send("Login request for " + login.user);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // 模拟业务逻辑判断
        bool is_success = (login.user == "user" && login.token == "123456");
        
        if (is_success) {
            Log::info("[LoginReceiver] Login successful.");
            if(ui) ui->showMessage("[System] Login successful!");
            // 【重要】发布成功事件，驱动状态机从 LoggingIn -> LoggedIn
            EventBus::GetInstance().publish(
                std::make_unique<LoginSuccessEvent>(login.user, login.token)
            );
        } else {
            Log::warn("[LoginReceiver] Login failed.");
            if(ui) ui->showMessage("[System] Login failed! Wrong password.");
            // 【重要】发布失败事件，驱动状态机从 LoggingIn -> LoggedOut
            EventBus::GetInstance().publish(
                std::make_unique<LoginFailedEvent>(login.user, "Invalid credentials")
            );
        }
    }

};