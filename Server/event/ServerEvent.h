//ServerEvent.h
#pragma once
#include <variant>
#include "ServerEvents.h"




using ServerEvent = std::variant<
    event::ErrorEvent,
    event::LoginRequest
    // event::Logout,
    // event::Chat,
    // 以后只需要往这里加
>;



