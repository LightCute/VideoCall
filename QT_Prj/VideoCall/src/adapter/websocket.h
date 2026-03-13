#pragma once
#include <memory>

#include "../service/abstract_net.h"
#include <rtc/rtc.hpp>

class WebSocket : public AbstractNet {
public:
    WebSocket();

    void connect(const std::string& url) override;
    void send(const std::string& msg) override;
    void close() override;

    void onMessage(MessageCallback cb) override;
    void onOpen(OpenCallback cb) override;
    void onClose(CloseCallback cb) override;
    void onError(ErrorCallback cb) override;

private:
    std::shared_ptr<rtc::WebSocket> ws;

    MessageCallback messageCallback;
    OpenCallback openCallback;
    CloseCallback closeCallback;
    ErrorCallback errorCallback;

    void initCallbacks();
};