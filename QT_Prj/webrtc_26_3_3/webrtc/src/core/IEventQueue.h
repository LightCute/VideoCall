// IEventQueue.h
#pragma once
#include <string>
#include <functional>
#include <eventpp/eventqueue.h>
#include "IEventData.h"
using EventQueue = eventpp::EventQueue<EventType, void(std::shared_ptr<IEventData>)>;
class IEventQueue {
public:
    virtual ~IEventQueue() = default;
    virtual void enqueue(EventType event,
        std::shared_ptr<IEventData> data) = 0;

    virtual void appendListener(
        EventType event,
        std::function<void(std::shared_ptr<IEventData>)> cb) = 0;
    virtual bool process() = 0;
};
