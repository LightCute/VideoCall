#pragma once
#include "../service/abstract_ui.h"
#include "../service/abstract_net.h"

class AppContext {
public:
    static AppContext& instance() {
        static AppContext ctx;
        return ctx;
    }

    AbstractUI* ui{};   // 仅 Receiver 访问
    AbstractNet* net{}; // 仅 Receiver 访问

private:
    AppContext() = default;
    ~AppContext() = default;
    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;
};