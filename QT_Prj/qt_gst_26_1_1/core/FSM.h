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
    EventType event_type; // 用 int 或枚举
    std::function<std::vector<CoreOutput>(const core::CoreInput&)> action;
    State next_state;
};

class FSM {
public:
    FSM(State init);

    std::vector<CoreOutput> handle(core::CoreInput ev);

    State getState() const { return state_; }

private:

    EventType eventTypeFromInput(const core::CoreInput& ev);

    State state_;
    std::vector<FSMEntry> table_;
    void initTable();
};
