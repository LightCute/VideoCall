#pragma once
#include <chrono>
#include <thread>
#include "../framework/event/event_bus.h"
#include "../service/abstract_ui.h"
#include "../utilities/log.h"
#include "../business/login/event/login_event.h"
class QtUI : public AbstractUI {
public:
    QtUI() {
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟用户操作延迟
            Log::info("[QtUI] QtUI constructor running in thread id: [{}]", Log::threadIdToString(std::this_thread::get_id()));

            EventBus::GetInstance().publish(
                std::make_unique<LoginEvent>("user", "123456", true)
            );
        }).detach();
    }
    void showMessage(const std::string& message) override {
        // Qt特定的UI显示逻辑
        // 这里仅模拟输出，实际应调用Qt的API显示消息
        Log::info("[QtUI] Displaying message: [{}]", message);
    }
};