// core/FSM.h
#pragma once
#include "CoreInput.h"
#include "CoreOutput.h"
#include "ClientState.h"
#include <vector>
#include <functional>
#include "ClientEventEnum.h"
#include <variant>
#include <iostream>

struct FSMEntry {
    State current_state;
    EventType event_type;
    // 补充 core:: 前缀
    std::function<std::vector<core::CoreOutput>(
        State current,
        const core::CoreInput&
        )> action;
    State next_state;
};

class FSM {
public:
    FSM();

    // 补充 core:: 前缀
    std::vector<core::CoreOutput>
    handle(State current, const core::CoreInput& ev);

private:
    EventType eventTypeFromInput(const core::CoreInput& ev);
    bool isOnlineState(State s);
    std::vector<FSMEntry> table_;
    void initTable();
};
