#pragma once
#include <memory>
#include "abstract_state.h"
#include "abstract_event.h"
#include "abstract_command.h"
#include "abstract_event_listener.h" // 新增：继承监听接口

class AbstractCommandDispatcher;

// 核心修改：继承AbstractEventListener，让所有会话具备事件监听能力
class AbstractSession : public AbstractEventListener {
public:
    virtual ~AbstractSession() = default;

    // 会话生命周期接口（保留）
    virtual void start() = 0;
    virtual void stop() = 0;

    // 保留：获取命令分发器（状态机构建命令时使用）
    AbstractCommandDispatcher* getDispatcher() const { return m_cmd_dispatcher; }
    
protected:
    std::unique_ptr<AbstractState> m_current_state;
    AbstractCommandDispatcher* m_cmd_dispatcher = nullptr;
};