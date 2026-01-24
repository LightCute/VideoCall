//protocol_common.h
#pragma once

namespace proto {

// commands
constexpr const char* CMD_LOGIN         = "LOGIN";
constexpr const char* CMD_LOGIN_OK      = "LOGIN_OK";
constexpr const char* CMD_LOGIN_FAIL    = "LOGIN_FAIL";
constexpr const char* CMD_ONLINE_USERS  = "ONLINE_USERS";
constexpr const char* CMD_HEARTBEAT     = "PING";
constexpr const char* CMD_HEARTBEAT_ACK = "PONG";
constexpr const char* CMD_REGISTER_PEER = "REGISTER_PEER";
constexpr const char* CMD_SEND_TEXT     = "SEND_TEXT";
constexpr const char* CMD_FORWARD_TEXT  = "FORWARD_TEXT";
// 通话相关CMD
constexpr const char* CMD_CALL               = "CALL";
constexpr const char* CMD_CALL_INCOMING      = "CALL_INCOMING";
constexpr const char* CMD_CALL_ACCEPT        = "CALL_ACCEPT";
constexpr const char* CMD_CALL_REJECT        = "CALL_REJECT";
constexpr const char* CMD_CALL_ACCEPTED      = "CALL_ACCEPTED";
constexpr const char* CMD_CALL_REJECTED      = "CALL_REJECTED";
constexpr const char* CMD_MEDIA_OFFER        = "MEDIA_OFFER";
constexpr const char* CMD_MEDIA_OFFER_RESP   = "MEDIA_OFFER_RESP";
constexpr const char* CMD_MEDIA_ANSWER       = "MEDIA_ANSWER";
constexpr const char* CMD_MEDIA_ANSWER_RESP  = "MEDIA_ANSWER_RESP";

// keys
constexpr const char* KEY_USERNAME  = "username";
constexpr const char* KEY_PASSWORD  = "password";
constexpr const char* KEY_PRIVILEGE = "privilege";
constexpr const char* KEY_MESSAGE   = "message";
constexpr const char* KEY_USERS     = "users";
constexpr const char* KEY_COUNT     = "count";

}
