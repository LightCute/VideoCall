#pragma once
#include <string>
#include "IEventData.h"

class MessageEvent : public IEventData {
public:
    MessageEvent(const std::string& msg, bool u) : message(msg), urgent(u) {}

    std::string getURL() {return message;}
private:
    std::string message;
    bool urgent;
};
