//protocol_common.h
#pragma once

namespace proto {

// commands
constexpr const char* CMD_LOGIN        = "LOGIN";
constexpr const char* CMD_LOGIN_OK     = "LOGIN_OK";
constexpr const char* CMD_LOGIN_FAIL   = "LOGIN_FAIL";
constexpr const char* CMD_ONLINE_USERS = "ONLINE_USERS";

// keys
constexpr const char* KEY_USERNAME  = "username";
constexpr const char* KEY_PASSWORD  = "password";
constexpr const char* KEY_PRIVILEGE = "privilege";
constexpr const char* KEY_MESSAGE   = "message";
constexpr const char* KEY_USERS     = "users";
constexpr const char* KEY_COUNT     = "count";

}
