#pragma once

#include "command_context.h"

// 接收者抽象：仅定义执行动作接口，所有业务接收者均继承此抽象
class AbstractReceiver {
public:
    virtual ~AbstractReceiver() = default;
    virtual void performAction(const CommandContext& context) = 0;
};