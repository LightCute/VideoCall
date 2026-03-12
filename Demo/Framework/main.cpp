#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "src/framework/core/core.h"
#include "src/framework/event/event_bus.h"
#include "src/framework/session/session_manager.h"
#include "src/business/login/session/login_session.h"
#include "src/framework/event/event_router.h" 
#include "src/adapter/qt_ui.h"
#include "src/adapter/websocket.h"
#include "src/application/app_context.h"
#include "src/utilities/log.h"
void simulateUI() {
    Log::info("[UI] User click login");
    EventBus::GetInstance().publish(
            std::make_unique<LoginEvent>("user", "123456", true)
        );
}

int main() {
    Log::init("app.log", Log::Mode::Async, spdlog::level::trace);
    Log::info("[Main] Starting application, thread id: [{}]", Log::threadIdToString(std::this_thread::get_id()));
    
    
    // 重构后初始化流程：EventRouter → EventBus → Core → SessionManager → Session
    // 核心逻辑：依赖注入，解耦各模块
    EventRouter router; // 1. 创建路由核心
    EventBus& eventBus = EventBus::GetInstance();
    eventBus.setRouter(&router); // 2. 关联EventBus和Router
    eventBus.start(); // 启动EventBus逻辑线程

    std::unique_ptr<AbstractCommandDispatcher> dispatcher = std::make_unique<Core>(); // 3. 创建命令执行核心
    auto sessionManager = std::make_unique<SessionManager>(&router); // 4. 创建会话管理器（注入路由）

    std::unique_ptr<AbstractUI> ui = std::make_unique<QtUI>();
    std::unique_ptr<AbstractNet> net = std::make_unique<WebSocket>();
    AppContext::instance().ui = ui.get();
    AppContext::instance().net = net.get();

    sessionManager->createSession<LoginSession>(dispatcher.get()); // 5. 创建会话（自动注册到路由）


    std::this_thread::sleep_for(std::chrono::seconds(1)); // 等待命令执行



    // std::thread simUiThread([]() {
    //     std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟用户操作延迟
    //     simulateUI(); // 触发登录事件
    // });
    std::this_thread::sleep_for(std::chrono::seconds(3)); // 等待命令执行

    // 优雅退出（SessionManager析构时自动注销监听者+停止会话）
    sessionManager.reset(); // 注销Session监听者
    dispatcher.reset();     // 停止Core线程
    eventBus.stop();        // 停止EventBus逻辑线程

    Log::info("Main: Exiting application");
    spdlog::shutdown();
    return 0;
}