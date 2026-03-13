#pragma once
#include <vector>
#include "../state/disconnected_state.h"
#include "../../../framework/session/abstract_session.h"
#include "../../../utilities/log.h"
#include "../event/connect_server_event.h"
#include "../event/connect_success_event.h"
#include "../event/connect_failed_event.h"
class ConnectSession : public AbstractSession {
public:
    explicit ConnectSession(AbstractCommandDispatcher* dispatcher) {
        m_cmd_dispatcher = dispatcher;
        Log::debug("[ConnectSession] Created");
    }

    // 核心：声明我只关心 LoginEvent
    std::vector<std::type_index> eventTypes() const override {
        return {
            typeid(ConnectServerEvent),  
            typeid(ConnectSuccessEvent),
            typeid(ConnectFailedEvent)
        };
    }

    // 实现IEventListener：处理LoginEvent（原有逻辑迁移）
    void handleEvent(const AbstractEvent& event) override {
        Log::debug("[ConnectSession] Handling event type: [{}]", typeid(event).name());
        if (!m_current_state) {
            Log::warn("[ConnectSession] No current state to handle event");
            return;
        }

        auto new_state = m_current_state->handleEvent(this, event);
        if (new_state) {
            Log::info("[ConnectSession] State transition triggered");
            m_current_state->onExit(this);
            m_current_state = std::move(new_state);
            m_current_state->onEnter(this);
        }
    }

    // 保留原有生命周期接口（仅修正状态初始化逻辑）
    void start() override {
        Log::info("[ConnectSession] Starting");
        if (m_current_state) {
            m_current_state->onExit(this);
        }
        m_current_state = std::make_unique<DisconnectedState>();
        m_current_state->onEnter(this);
    }

    void stop() override {
        Log::info("[ConnectSession] Stopping");
        if (m_current_state) {
            m_current_state->onExit(this);
            m_current_state.reset();
        }
    }
};
