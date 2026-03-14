#include <thread>
#include <QApplication>
#include <rtc/rtc.hpp>
#include "widget.h"
#include "utilities/log.h"
#include "socket.h"
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
    Log::init("app.log", Log::Mode::Async, spdlog::level::trace);
    Log::info("[Main] Starting application, thread id: [{}]", Log::threadIdToString(std::this_thread::get_id()));
    rtc::InitLogger(rtc::LogLevel::Debug, myCppLogCallback);
    WebSocket ws;
    
    Widget w(&ws);
    w.show();
    return a.exec();
}
