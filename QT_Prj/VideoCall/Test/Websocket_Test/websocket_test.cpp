#include "../../src/adapter/websocket.h"
#include <iostream>
#include <thread>

int main() {

    rtc::InitLogger(rtc::LogLevel::Verbose);

    WebSocket ws;

    ws.onOpen([](){
        std::cout << "WebSocket connected" << std::endl;
    });

    ws.onMessage([](const std::string& msg){
        std::cout << "Received: " << msg << std::endl;
    });

    ws.onClose([](){
        std::cout << "WebSocket closed" << std::endl;
    });

    ws.onError([](const std::string& err){
        std::cout << "Error: " << err << std::endl;
    });

    ws.connect("ws://localhost:8000");

    std::cout << "Press enter to send message" << std::endl;
    std::cin.get();

    ws.send("Hello Server");

    std::cout << "Press enter to exit" << std::endl;
    std::cin.get();

    return 0;
}