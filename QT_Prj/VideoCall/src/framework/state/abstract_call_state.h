
// abstract_call_state.h
#pragma once
#include "abstract_state.h"

class AbstractCallState : public AbstractState {
public:
    ~AbstractCallState() override = default;
    // 可扩展通话状态通用接口，不修改抽象核心逻辑
};