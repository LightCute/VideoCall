// abstract_login_state.h
#pragma once
#include "abstract_state.h"

class AbstractLoginState : public AbstractState {
public:
    ~AbstractLoginState() override = default;
    // 可扩展登录状态通用接口，不修改抽象核心逻辑
};

