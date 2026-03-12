#pragma once
#include <vector>
#include <typeindex>
#include "abstract_event.h"

class AbstractEventListener {
public:
    virtual ~AbstractEventListener() = default;

    virtual std::vector<std::type_index> eventTypes() const = 0;

    virtual void handleEvent(const AbstractEvent& event) = 0;
};