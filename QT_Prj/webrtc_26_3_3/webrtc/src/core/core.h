#pragma once
#include <eventpp/eventqueue.h>
#include <string>
#include "IEventQueue.h"

class Core : public IEventQueue {
public:

    void enqueue(EventType event,
                 std::shared_ptr<IEventData> data) override
    {
        queue.enqueue(event, data);
    }

    void appendListener(
        EventType event,
        std::function<void(std::shared_ptr<IEventData>)> cb) override
    {
        queue.appendListener(event, cb);
    }

    bool process() override
    {
        return queue.process();
    }

private:
    EventQueue queue;
};
