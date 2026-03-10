#pragma once

#include <vector>
#include <memory>
#include <string>
#include <any>
#include <stdexcept>

// 通用命令上下文：解耦命令与业务数据，扩展无需修改抽象
class CommandContext {
public:
    CommandContext() = default;

    template<typename T>
    CommandContext(T data)
        : payload(std::move(data)) {}

    template<typename T>
    void set(T data)
    {
        payload = std::move(data);
    }

    template<typename T>
    T& get()
    {
        return std::any_cast<T&>(payload);
    }

    template<typename T>
    const T& get() const
    {
        return std::any_cast<const T&>(payload);
    }

public:
    std::string session_id;
    uint64_t timestamp = 0;

private:
    std::any payload;
};

struct LoginPayload
{
    std::string user;
    std::string token;
};

// 接收者抽象：仅定义执行动作接口，所有业务接收者均继承此抽象
class AbstractReceiver {
public:
    virtual ~AbstractReceiver() = default;
    virtual void performAction(const CommandContext& context) = 0;
};

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

class AbstractCommandDispatcher
{
public:
    virtual ~AbstractCommandDispatcher() = default;

    virtual void postCommand(
        std::unique_ptr<AbstractCommand> cmd
    ) = 0;
};

