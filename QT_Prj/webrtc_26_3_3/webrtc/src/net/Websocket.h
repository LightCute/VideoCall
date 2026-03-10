#pragma once
#include <rtc/rtc.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include "INet.h"
#include "../core/IEventQueue.h"
class WebSocketClient : public INet
{
public:
    WebSocketClient(IEventQueue* queue);
    ~WebSocketClient();

    void connect(const std::string& url) override;
    void send(const std::string& msg) override;
    void close() override;


private:
    void run();

private:
    std::shared_ptr<rtc::WebSocket> ws;

    std::thread networkThread;
    std::atomic<bool> running{false};

    IEventQueue* m_queue;
};
