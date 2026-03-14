#pragma once
#include "../../../framework/command/abstract_receiver.h"
#include "../../../framework/command/command_context.h"
#include "../model/send_payload.h"
#include "../../../utilities/log.h"
#include "../../../service/abstract_ui.h"
#include "../../../service/abstract_net.h"
#include "../../../application/app_context.h"
#include "../../../framework/event/event_bus.h"

class CallSendReceiver : public AbstractReceiver {
public:
    void performAction(const CommandContext& context) override {
        auto& call = context.get<SendPayload>();
        Log::info("[CallSendReceiver] Processing call send msg = [{}]", 
                  call.msg);

        auto* ui = AppContext::instance().ui;
        auto* net = AppContext::instance().net;

        if(ui) ui->showMessage("[System] Sending message...");
        if(net) {
            net->send2Peer("", call.msg); // 目前peerId未使用，后续可根据实际需求调整CallSendEvent和SendPayload结构
        }


    }

};