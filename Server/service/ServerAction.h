// Service/ServerAction.h
#pragma once
#include <variant>
#include "ServerActions.h"

using ServerAction = std::variant<
    SendError,
    SendLoginOk,
    SendLoginFail,
    BroadcastOnlineUsers,
    BroadcastLogout,
    SendHeartbeatAck,
    UpdatePeerInfo,
    ForwardText,
    SendUserNotFound,
    SendCallIncoming,
    SendCallAccepted,
    SendCallRejected
>;
