#pragma once
#include <memory>
#include "../state/abstract_state.h"
#include "../command/abstract_command_dispatcher.h"
#include "../event/abstract_event_listener.h"

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