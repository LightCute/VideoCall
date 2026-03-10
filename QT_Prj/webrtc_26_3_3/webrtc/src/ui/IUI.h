#pragma once
#include <string>

class IUI
{
public:
    virtual ~IUI() = default;

    virtual void showMessage(const std::string& msg) = 0;

    //virtual void run() = 0;
};
