#pragma once
#include <eventpp/eventqueue.h>
#include <string>
#include "IEventQueue.h"
#include "../utilities/log.h"

class Core : public IEventQueue {
public:

    void enqueue(EventType event,
                 std::shared_ptr<IEventData> data) override
    {
        Log::debug("[CORE] enqueue event {}", data->getEventName());
        queue.enqueue(event, data);
    }

    void appendListener(
        EventType event,
        std::function<void(std::shared_ptr<IEventData>)> cb) override
    {
        Log::debug("[CORE] appendListener event {}", (int)event);
        queue.appendListener(event, cb);
    }

    bool process() override
    {
        Log::trace("[CORE] processing events");
        return queue.process();
    }

private:
    EventQueue queue;
};
