#pragma once
#include <string>
#include "../../../framework/event/abstract_event.h"


class ConnectServerEvent : public AbstractEvent {
public:

    ConnectServerEvent(const std::string& url)
        : m_url(url) {}

    // 只读getter，保证事件数据不可变，线程安全
    const std::string& getUrl() const { return m_url; }

private:
    const std::string m_url;
};


