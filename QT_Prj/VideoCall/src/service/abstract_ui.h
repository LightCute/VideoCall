#pragma once 
#include <string>

class AbstractUI {
public:
    virtual ~AbstractUI() = default;
    virtual void showMessage(const std::string& message) = 0;
    virtual void showUI() = 0;
};