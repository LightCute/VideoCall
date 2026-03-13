// abstract_connect_state.h
#pragma once
#include "abstract_state.h"

class AbstractConnectState : public AbstractState {
public:
    ~AbstractConnectState() override = default;
    // 可扩展连接状态通用接口，不修改抽象核心逻辑
};

