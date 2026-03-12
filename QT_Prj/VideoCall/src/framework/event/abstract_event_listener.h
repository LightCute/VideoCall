#pragma once
#include <typeindex>
#include "abstract_event.h"

class AbstractEventListener {
public:
    virtual ~AbstractEventListener() = default;

    virtual std::type_index eventType() const = 0;

    virtual void handleEvent(const AbstractEvent& event) = 0;
};