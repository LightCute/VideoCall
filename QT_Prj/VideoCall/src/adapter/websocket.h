#pragma once 
#include "../service/abstract_net.h"
#include "../utilities/log.h"
class WebSocket : public AbstractNet {
public:
    void send(const std::string& msg) override {
        // WebSocket特定的发送逻辑
        // 这里仅模拟输出，实际应调用WebSocket的API发送消息  
        Log::info("[WebSocket] Sending message: [{}]", msg);       
    }
    void connect(const std::string& address) override {     
        // WebSocket特定的连接逻辑
        // 这里仅模拟输出，实际应调用WebSocket的API连接服务器
        Log::info("[WebSocket] Connecting to address: [{}]", address);
    }
};