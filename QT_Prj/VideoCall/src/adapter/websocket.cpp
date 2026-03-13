#include "websocket.h"
#include "../framework/event/event_bus.h"
#include "../business/connect/event/connect_success_event.h"
#include "../business/connect/event/connect_failed_event.h"

WebSocket::WebSocket() {
    ws = std::make_shared<rtc::WebSocket>();
    initCallbacks();
}

void WebSocket::initCallbacks() {

    ws->onOpen([this]() {
        EventBus::GetInstance().publish(
            std::make_unique<ConnectSuccessEvent>()
            );
    });

    ws->onClosed([this]() {
        // EventBus::GetInstance().publish(
        //     std::make_unique<ConnectFailedEvent>()
        //     );
    });

    ws->onError([this](std::string err) {
        EventBus::GetInstance().publish(
            std::make_unique<ConnectFailedEvent>()
            );
    });

    ws->onMessage([this](auto data) {

        if (!std::holds_alternative<std::string>(data))
            return;

        if (messageCallback)
            messageCallback(std::get<std::string>(data));
    });
}

void WebSocket::connect(const std::string& url) {
    ws->open(url);
}

void WebSocket::send(const std::string& msg) {
    ws->send(msg);
}

void WebSocket::close() {
    ws->close();
}

void WebSocket::onMessage(MessageCallback cb) {
    messageCallback = cb;
}

void WebSocket::onOpen(OpenCallback cb) {
    openCallback = cb;
}

void WebSocket::onClose(CloseCallback cb) {
    closeCallback = cb;
}

void WebSocket::onError(ErrorCallback cb) {
    errorCallback = cb;
}