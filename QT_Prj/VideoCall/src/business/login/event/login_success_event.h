#pragma once
#include <string>
#include "../../../framework/event/abstract_event.h"

class LoginSuccessEvent : public AbstractEvent {
public:
    explicit LoginSuccessEvent(const std::string& user_id, const std::string& token)
        : m_user_id(user_id), m_token(token) {}

    const std::string& getUserId() const { return m_user_id; }
    const std::string& getToken() const { return m_token; }

private:
    const std::string m_user_id;
    const std::string m_token;
};