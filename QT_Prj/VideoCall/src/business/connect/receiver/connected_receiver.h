#pragma once
#include "../../../framework/command/abstract_receiver.h"
#include "../../../framework/command/command_context.h"
#include "../model/connect_payload.h"
#include "../../../utilities/log.h"
#include "../../../service/abstract_ui.h"
#include "../../../service/abstract_net.h"
#include "../../../application/app_context.h"
#include "../../../framework/event/event_bus.h"

class ConnectedReceiver : public AbstractReceiver {
public:
    void performAction(const CommandContext& context) override {
        Log::info("[ConnectedReceiver] Processing connected ");

        auto* ui = AppContext::instance().ui;
        auto* net = AppContext::instance().net;

        if(ui) ui->showMessage("[System] Connected!");
        if(net) {
            
        }


    }

};