#pragma once
#include "../../framework/event/abstract_event.h"
#include <string>

class LoginEvent : public AbstractEvent {
public:

    LoginEvent(const std::string& user_id, const std::string& token, bool is_success)
        : m_user_id(user_id), m_token(token), m_is_success(is_success) {}

    // 只读getter，保证事件数据不可变，线程安全
    const std::string& getUserId() const { return m_user_id; }
    const std::string& getToken() const { return m_token; }
    bool isSuccess() const { return m_is_success; }

private:
    const std::string m_user_id;
    const std::string m_token;
    const bool m_is_success;
};


