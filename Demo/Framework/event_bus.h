#pragma once
#include "abstract_event.h"
#include "event_router.h"
#include "blocking_queue.h"
#include "log.h"
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <memory>

// 异步EventBus核心：接收事件入队 + 逻辑线程处理
class EventBus {
public:
    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    // 关联路由（依赖注入，支持测试/扩展）
    void setRouter(EventRouter* router) {
        m_router = router;
        Log::debug("EventBus: Router set");
    }

    // 启动逻辑线程（必须在发布事件前调用）
    void start() {
        if (m_running) return;
        m_running = true;
        // 启动逻辑线程：处理队列中的事件
        m_logic_thread = std::thread(&EventBus::logicLoop, this);
        Log::info("EventBus: Logic thread started, id: {}", Log::threadIdToString(m_logic_thread.get_id()));
    }

    // 停止逻辑线程（优雅退出）
    void stop() {
        if (!m_running) return;
        m_running = false;
        m_event_queue.stop(); // 唤醒队列阻塞的线程
        if (m_logic_thread.joinable()) {
            m_logic_thread.join(); // 等待线程退出
        }
        Log::info("EventBus: Logic thread exited.");
    }

    // 异步发布事件：接收unique_ptr，转移事件所有权（线程安全）
    void publish(std::unique_ptr<AbstractEvent> event) {
        if (!event) {
            Log::warn("EventBus: Attempt to publish null event");
            return;
        }
        Log::trace("EventBus: Publishing event type: {}", typeid(*event).name());
        m_event_queue.push(std::move(event)); // 事件入队，立即返回
    }

private:
    EventBus() : m_running(false) {}
    ~EventBus() {
        stop(); // 析构时自动停止
    }

    // 逻辑线程主循环：持续消费事件队列
    void logicLoop() {
        Log::debug("EventBus: Logic loop started");
        while (m_running) {
            // 阻塞获取事件（队列为空时等待，stop时返回空）
            auto event = m_event_queue.waitAndPop();
            if (!event || !m_running) {
                continue;
            }
            Log::trace("EventBus: Routing event type: {}", typeid(*event).name());
            // 路由事件（仅在逻辑线程执行）
            if (m_router) {
                m_router->route(*event); // 调用Router分发事件
            }
        }
        Log::debug("EventBus: Logic loop exited");
    }

    // 核心成员变量
    EventRouter* m_router = nullptr;                // 事件路由接口
    BlockingQueue<std::unique_ptr<AbstractEvent>> m_event_queue; // 事件阻塞队列
    std::thread m_logic_thread;                     // 事件处理逻辑线程
    std::atomic<bool> m_running;                    // 线程运行标志（原子变量，线程安全）
};