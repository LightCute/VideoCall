#pragma once
#include <string>
#include "../../../framework/event/abstract_event.h"


class CallEvent : public AbstractEvent {
public:

    CallEvent(const std::string& peer_id)
        : m_peer_id(peer_id) {}

    // 只读getter，保证事件数据不可变，线程安全
    const std::string& getPeerid() const { return m_peer_id; }

private:
    const std::string m_peer_id;
};


