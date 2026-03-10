#pragma once
#include "abstract_command.h"


class LoginReceiver : public AbstractReceiver {
public:
    void performAction(const CommandContext& context) override {

        auto& login = context.get<LoginPayload>();

        Log::info("LoginReceiver: Processing login - user = {}, token = {}, timestamp = {}", 
                  login.user, login.token, context.timestamp);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        Log::info("LoginReceiver: Login completed for user: {}", login.user);
    }
};