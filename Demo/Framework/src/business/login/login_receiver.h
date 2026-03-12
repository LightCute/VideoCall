#pragma once
#include <thread>
#include "../../framework/command/abstract_receiver.h"
#include "../../framework/command/command_context.h"
#include "login_payload.h"
#include "../../utilities/log.h"
#include "../../service/abstract_ui.h"
#include "../../service/abstract_net.h"
#include "../../application/app_context.h"

class LoginReceiver : public AbstractReceiver {
public:
    void performAction(const CommandContext& context) override {

        auto& login = context.get<LoginPayload>();



        Log::info("[LoginReceiver] Processing login - user = [{}], token = [{}], timestamp = [{}]", 
                  login.user, login.token, context.timestamp);

                auto* ui = AppContext::instance().ui;
        auto* net = AppContext::instance().net;

        if(ui) ui->showMessage("[LoginReceiver] Logging in user: " + login.user);
        if(net) net->send("Login request for " + login.user);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        Log::info("[LoginReceiver] Login completed for user: [{}]", login.user);
    }

};