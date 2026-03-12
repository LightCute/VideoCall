#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include "abstract_session.h"
#include "../event/event_router.h" // 新增：依赖路由，不再依赖EventBus

// 核心修改：移除EventBus依赖，改为依赖EventRouter
// 价值：SessionManager不再耦合具体事件类型，符合OCP
class SessionManager {
public:
    // 构造函数：注入路由（而非EventBus）
    explicit SessionManager(EventRouter* router) : m_router(router) {
        Log::debug("[SessionManager] Created");
    }

    ~SessionManager() {
        Log::info("[SessionManager] Destroying, cleaning up [{}] sessions",
             m_sessions.size());
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& session : m_sessions) {
            if (m_router) {
                // 遍历事件类型列表，逐个注销
                auto event_types = session->eventTypes();
                for (auto& event_type : event_types) {
                    Log::debug("[SessionManager] Unregistering listener for event type: [{}]", 
                        event_type.name());
                    m_router->unregisterListener(event_type, session.get());
                }
            }
            session->stop();
        }
        m_sessions.clear();
    }

    // 保留模板创建逻辑：但新增「注册到路由」的逻辑
    template<typename SessionType, typename... Args>
    void createSession(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto session = std::make_unique<SessionType>(std::forward<Args>(args)...);
        
        // 核心：会话创建后自动注册到路由（无需硬编码事件类型）
        if (m_router) {
            auto event_types = session->eventTypes();
            for (auto& event_type : event_types) {
                Log::info("[SessionManager] Registering session for event type: [{}]", 
                    event_type.name());
                m_router->registerListener(event_type, session.get());
            }
        }
        
        session->start();
        m_sessions.push_back(std::move(session));
        Log::info("[SessionManager] Session created, total sessions: [{}]",
            m_sessions.size());
    }

private:
    EventRouter* m_router; // 依赖路由接口，而非具体事件
    std::vector<std::unique_ptr<AbstractSession>> m_sessions;
    mutable std::mutex m_mutex; // 保护会话列表的线程安全
};