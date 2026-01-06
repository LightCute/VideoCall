//ServerEventFactory.h
#pragma once
#include <string>
#include "./event/ServerEvent.h"

class ServerEventFactory {
public:
    static ServerEvent makeEvent(const std::string& msg);
};
