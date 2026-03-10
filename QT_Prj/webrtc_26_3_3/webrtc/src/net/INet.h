#pragma once
#include <string>
#include <functional>

class INet
{
public:

    virtual ~INet() = default;

    virtual void connect(const std::string& url) = 0;

    virtual void send(const std::string& msg) = 0;

    virtual void close() = 0;

};
