#pragma once
#include <memory>
#include "abstract_event.h"

// 前置声明，避免循环依赖
class AbstractSession;

class AbstractState {
public:
    virtual ~AbstractState() = default;

    // 进入状态时执行：初始化、状态事件发布等
    virtual void onEnter(AbstractSession* session) = 0;
    // 处理事件，返回新状态（需切换状态时返回实例，否则返回nullptr）
    virtual std::unique_ptr<AbstractState> handleEvent(
        AbstractSession* session, const AbstractEvent& event) = 0;
    // 退出状态时执行：资源清理、状态事件发布等
    virtual void onExit(AbstractSession* session) = 0;
};

