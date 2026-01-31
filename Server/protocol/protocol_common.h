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
constexpr const char* CMD_REGISTER_PEER = "REGISTER_PEER";
constexpr const char* CMD_SEND_TEXT     = "SEND_TEXT";       // 客户端→服务端：发送文本
constexpr const char* CMD_FORWARD_TEXT  = "FORWARD_TEXT"; // 服务端→客户端：转发文本

constexpr const char* CMD_CALL         = "CALL";         // 客户端发起呼叫
constexpr const char* CMD_CALL_ACCEPT  = "CALL_ACCEPT";  // 客户端接受呼叫
constexpr const char* CMD_CALL_REJECT  = "CALL_REJECT";  // 客户端拒绝呼叫
constexpr const char* CMD_CALL_INCOMING = "CALL_INCOMING"; // 服务端通知新来电
constexpr const char* CMD_CALL_ACCEPTED = "CALL_ACCEPTED"; // 服务端通知通话接通
constexpr const char* CMD_CALL_REJECTED = "CALL_REJECTED"; // 服务端通知通话被拒

// 媒体协商指令
constexpr const char* CMD_MEDIA_OFFER  = "MEDIA_OFFER";   // 客户端→服务端：发起媒体协商
constexpr const char* CMD_MEDIA_ANSWER = "MEDIA_ANSWER";  // 客户端→服务端：回应媒体协商
constexpr const char* CMD_MEDIA_OFFER_RESP  = "MEDIA_OFFER_RESP";  // 服务端→客户端：下发对方IP（Offer回应）
constexpr const char* CMD_MEDIA_ANSWER_RESP = "MEDIA_ANSWER_RESP"; // 服务端→客户端：下发对方IP（Answer回应

// 在通话相关CMD后新增
constexpr const char* CMD_CALL_HANGUP    = "CALL_HANGUP";    // 客户端→服务端：主动挂断请求
constexpr const char* CMD_CALL_ENDED     = "CALL_ENDED";     // 服务端→客户端：通知对端挂断（携带原因）

// keys
constexpr const char* KEY_USERNAME  = "username";
constexpr const char* KEY_PASSWORD  = "password";
constexpr const char* KEY_PRIVILEGE = "privilege";
constexpr const char* KEY_MESSAGE   = "message";
constexpr const char* KEY_USERS     = "users";
constexpr const char* KEY_COUNT     = "count";



}
