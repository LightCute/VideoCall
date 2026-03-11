#include "src/ui/widget.h"

#include <QApplication>
#include <iostream>
#include <rtc/rtc.hpp>
#include "eventpp/eventdispatcher.h"
#include "./src/net/Websocket.h"
#include "./src/core/core.h"
#include "src/utilities/log.h"

void myCppLogCallback(rtc::LogLevel level, std::string message) {
    switch (level) {
    case rtc::LogLevel::Fatal:
    case rtc::LogLevel::Error:
        Log::error("[RTC] {}", message);
        break;
    case rtc::LogLevel::Warning:
        Log::warn("[RTC] {}", message);
        break;
    case rtc::LogLevel::Info:
        Log::info("[RTC] {}", message);
        break;
    case rtc::LogLevel::Debug:
        Log::debug("[RTC] {}", message);
        break;
    case rtc::LogLevel::Verbose:
        Log::trace("[RTC] {}", message);
        break;
    default:
        Log::info("[RTC] {}", message);
        break;
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Log::init("webrtc.log", Log::Mode::Async, spdlog::level::trace);
    Log::info("[APP] Application starting");

    rtc::InitLogger(rtc::LogLevel::Verbose, myCppLogCallback);
    Log::info("[APP] RTC logger initialized");

    Core core;
    Log::info("[APP] Core created");

    WebSocketClient net(&core);
    Log::info("[APP] WebSocketClient created");

    Widget w(&core);
    Log::info("[APP] Widget created");

    w.show();
    Log::info("[APP] Entering Qt event loop");

    return a.exec();
}
