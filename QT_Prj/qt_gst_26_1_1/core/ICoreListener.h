// core/ICoreListener.h
#pragma once
#include "CoreOutput.h"

namespace core {

class ICoreListener {
public:
    virtual ~ICoreListener() = default;

    // Core 输出事件的统一回调（Core 线程调用）
    virtual void onCoreOutput(const CoreOutput& out) = 0;
};

} // namespace core
