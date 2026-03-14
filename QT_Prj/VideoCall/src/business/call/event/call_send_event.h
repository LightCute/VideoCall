#pragma once
#include <string>
#include "../../../framework/event/abstract_event.h"


class CallSendEvent : public AbstractEvent {
public:

    CallSendEvent(const std::string& msg)
        :  m_msg(msg) {}

    // 只读getter，保证事件数据不可变，线程安全
    const std::string& getPeerid() const { return m_peer_id; }
    const std::string& getMsg() const { return m_msg; }

private:
    const std::string m_peer_id;
    const std::string m_msg;
};


