// Service/ServerAction.h
#pragma once
#include <variant>
#include "ServerActions.h"

using ServerAction = std::variant<
    SendError,
    SendLoginOk,
    SendLoginFail,
    BroadcastOnlineUsers
>;
