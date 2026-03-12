#pragma once
#include <string>
#include "../../framework/event/abstract_event.h"

class LoginFailedEvent : public AbstractEvent {
public:
    explicit LoginFailedEvent(const std::string& user_id, const std::string& error_msg)
        : m_user_id(user_id), m_error_msg(error_msg) {}

    const std::string& getUserId() const { return m_user_id; }
    const std::string& getErrorMsg() const { return m_error_msg; }

private:
    const std::string m_user_id;
    const std::string m_error_msg;
};
