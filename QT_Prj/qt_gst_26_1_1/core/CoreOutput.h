// core/CoreOutput.h
#pragma once
#include <variant>
#include "ClientState.h"

struct OutStateChanged { State state; };
struct OutLoginOk {};
struct OutLoginFail { std::string msg; };
struct OutDisconnected {};
struct OutConnect { std::string host; int port; };
struct OutSendLogin { std::string user; std::string pass; };

using CoreOutput = std::variant<
    OutStateChanged,
    OutLoginOk,
    OutLoginFail,
    OutDisconnected,
    OutConnect,
    OutSendLogin
    >;
