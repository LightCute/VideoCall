// core/CoreOutput.h
#pragma ocne
#include <variant>
#include "ClientState.h"
struct OutStateChanged { State state; };
struct OutLoginOk {};
struct OutLoginFail { std::string msg; };
struct OutDisconnected {};

using CoreOutput = std::variant<
    OutStateChanged,
    OutLoginOk,
    OutLoginFail,
    OutDisconnected
    >;
