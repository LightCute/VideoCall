#pragma once
#include <any>
#include <string>
#include <cstdint>
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
