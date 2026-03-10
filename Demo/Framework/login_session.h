#pragma once
#include "abstract_session.h"
#include "abstract_command.h"
#include "logged_in_state.h"
#include "login_command.h"
#include "login_receiver.h"
#include "login_event.h" // 新增：获取LoginEvent类型

// 核心修改：实现IEventListener接口，声明仅关心LoginEvent
// 价值：精确投递的关键——只有LoginSession会接收LoginEvent
class LoginSession : public AbstractSession {
public:
    explicit LoginSession(AbstractCommandDispatcher* dispatcher) {
        m_cmd_dispatcher = dispatcher;
        Log::debug("LoginSession: Created");
    }

    // 核心：声明我只关心 LoginEvent
    std::type_index eventType() const override {
        return typeid(LoginEvent);
    }

    // 实现IEventListener：处理LoginEvent（原有逻辑迁移）
    void handleEvent(const AbstractEvent& event) override {
        Log::debug("LoginSession: Handling event type: {}", typeid(event).name());
        if (!m_current_state) {
            Log::warn("LoginSession: No current state to handle event");
            return;
        }

        auto new_state = m_current_state->handleEvent(this, event);
        if (new_state) {
            Log::info("LoginSession: State transition triggered");
            m_current_state->onExit(this);
            m_current_state = std::move(new_state);
            m_current_state->onEnter(this);
        }
    }

    // 保留原有生命周期接口（仅修正状态初始化逻辑）
    void start() override {
        Log::info("LoginSession: Starting");
        if (m_current_state) {
            m_current_state->onExit(this);
        }
        m_current_state = std::make_unique<LoggedOutState>();
        m_current_state->onEnter(this);
    }

    void stop() override {
        Log::info("LoginSession: Stopping");
        if (m_current_state) {
            m_current_state->onExit(this);
            m_current_state.reset();
        }
    }
};