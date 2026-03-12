#pragma once

#include <vector>
#include <memory>
#include <string>
#include <stdexcept> // 仅使用std::invalid_argument
#include "abstract_receiver.h"
#include "command_context.h"




// 命令抽象：仅定义命令的核心能力，具体业务命令仅需继承实现
class AbstractCommand {
public:    
    virtual ~AbstractCommand() = default;
    // 核心执行接口，返回执行结果，支持异常传递
    virtual bool execute() = 0;
    // 命令类型标识：用于监控、过滤、扩展，无需修改核心调度逻辑
    virtual std::string getCommandType() const = 0;

    // 接收者绑定逻辑通用实现，子类无需修改
    void addReceiver(std::shared_ptr<AbstractReceiver> receiver) {
        if (!receiver) throw std::invalid_argument("receiver is null");
        m_receivers.push_back(std::move(receiver));
    }

protected:
    CommandContext m_context; // 可选：命令上下文，供子类使用
    std::vector<std::shared_ptr<AbstractReceiver>> m_receivers;
};



