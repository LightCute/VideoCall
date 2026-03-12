#pragma once
#include <string>

class AbstractNet {
public:
    virtual ~AbstractNet() = default;
    virtual void send(const std::string& msg) = 0;
    virtual void connect(const std::string& address) = 0;

};