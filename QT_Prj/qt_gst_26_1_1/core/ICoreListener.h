// core/ICoreListener.h
#pragma once
#include "CoreOutput.h"

namespace core {

class ICoreListener {
public:
    virtual ~ICoreListener() = default;

    // ========== 关键修改：将 onCoreOutput 改为 onUiOutput，参数改为 UiOutput ==========
    // UI 层仅能接收 UiOutput，无法访问任何 Executor 相关类型
    virtual void onUiOutput(const UiOutput& out) = 0;
};

} // namespace core
