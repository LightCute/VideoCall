// proto::*
// ⚠️ PROTOCOL DTO — do NOT use as domain model
// ⚠️ 仅用于协议序列化 / 反序列化
// ⚠️ 禁止在业务层、事件层、服务层中作为业务模型使用

//protocol_common.h

#pragma once

namespace proto {

// commands
constexpr const char* CMD_LOGIN        = "LOGIN";
constexpr const char* CMD_LOGIN_OK     = "LOGIN_OK";
constexpr const char* CMD_LOGIN_FAIL   = "LOGIN_FAIL";
constexpr const char* CMD_ONLINE_USERS = "ONLINE_USERS";
constexpr const char* CMD_LOGOUT       = "LOGOUT";
constexpr const char* CMD_HEARTBEAT = "PING";
constexpr const char* CMD_HEARTBEAT_ACK = "PONG";

// keys
constexpr const char* KEY_USERNAME  = "username";
constexpr const char* KEY_PASSWORD  = "password";
constexpr const char* KEY_PRIVILEGE = "privilege";
constexpr const char* KEY_MESSAGE   = "message";
constexpr const char* KEY_USERS     = "users";
constexpr const char* KEY_COUNT     = "count";

}
