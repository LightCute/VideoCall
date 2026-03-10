#pragma once
#include <unordered_map>
#include <vector>
#include <shared_mutex>
#include <algorithm>
#include <mutex>
#include <typeindex>
#include "abstract_event_listener.h"
#include "abstract_event.h"
#include "log.h"

// 事件路由核心：维护「事件类型→监听者」映射，实现精确投递
// 核心价值：替代原SessionManager的广播逻辑，解决冗余事件分发问题
class EventRouter {
public:
    // 注册监听者：关联事件类型和处理者（线程安全）
    void registerListener(std::type_index event_type, AbstractEventListener* listener) {
        if (!listener) {
            Log::warn("EventRouter: Attempt to register null listener");
            return;
        }
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_listeners[event_type].push_back(listener);
        Log::debug("EventRouter: Listener registered for event type: {}, total listeners: {}", 
               event_type.name(),  
               m_listeners[event_type].size());
    }

    // 注销监听者：避免野指针（Session销毁时调用）
    void unregisterListener(std::type_index event_type, AbstractEventListener* listener) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_listeners.find(event_type);
        if (it == m_listeners.end()) {
            Log::debug("EventRouter: No listeners found for event type: {} during unregister", 
                event_type.name());
            return;
        }
        auto& listeners = it->second;
        Log::debug("EventRouter: Unregistering listener for event type: {}",
            event_type.name());
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
    }

    // 路由事件：仅投递到关心该事件的监听者（精确投递核心）
    void route(const AbstractEvent& event) {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_listeners.find(typeid(event));
        if (it == m_listeners.end()) {
            Log::trace("EventRouter: No listeners for event type: {}", typeid(event).name());
            return;
        }

        // 拷贝监听者列表：避免执行时注销导致迭代器失效（线程安全）
        auto listeners = it->second;
        lock.unlock();
        Log::trace("EventRouter: Routing event type: {} to {} listeners", 
            typeid(event).name(), listeners.size());
        // 仅调用关心该事件的监听者
        for (auto listener : listeners) {
            if (listener) {
                listener->handleEvent(event);
            }
        }
    }

private:
    // 事件类型 → 监听者列表（核心映射表）
    std::unordered_map<std::type_index, std::vector<AbstractEventListener*>> m_listeners;
    // 读写锁：支持多线程并发注册/路由（读多写少场景最优）
    mutable std::shared_mutex m_mutex;
};