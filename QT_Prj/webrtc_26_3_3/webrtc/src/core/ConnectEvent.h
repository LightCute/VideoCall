#pragma once
#include <string>
#include "IEventData.h"

class MessageEvent : public IEventData {
public:
    MessageEvent(const std::string& msg, bool u) : message(msg), urgent(u) {}

    std::string getURL() {return message;}
    std::string getEventName() const override { return "MessageEvent"; }
private:
    std::string message;
    bool urgent;
};
