#include "Websocket.h"
#include <iostream>
#include "../core/ConnectEvent.h"
WebSocketClient::WebSocketClient(IEventQueue* queue)
    : m_queue( queue)
{
    //rtc::InitLogger(rtc::LogLevel::Debug);

    m_queue->appendListener(
        EventType::UI_NetConnectServer,
        [this](std::shared_ptr<IEventData> data)
        {
            auto msg = std::dynamic_pointer_cast<MessageEvent>(data);
            if(msg)
                connect(msg->getURL());
        }
    );
}

WebSocketClient::~WebSocketClient()
{
    close();
}

void WebSocketClient::connect(const std::string& url)
{
    running = true;

    networkThread = std::thread([this, url]()
    {
        ws = std::make_shared<rtc::WebSocket>();

        ws->onOpen([]()
        {
            std::cout << "WebSocket connected\n";
        });

        ws->onClosed([]()
        {
            std::cout << "WebSocket closed\n";
        });

        ws->onMessage([this](rtc::message_variant msg)
        {

        });

        ws->open(url);

        while(running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void WebSocketClient::send(const std::string& msg)
{
    if(ws)
        ws->send(msg);
}

void WebSocketClient::close()
{
    running = false;

    if(ws)
        ws->close();

    if(networkThread.joinable())
        networkThread.join();
}


// void WebSocketClient::onMessage(MessageCallback cb)
// {
//     ws->onMessage([this](rtc::message_variant msg)
//                   {
//                       if(auto str = std::get_if<std::string>(&msg))
//                       {
//                           m_queue->enqueue(
//                               EventType::Net_RecvMessage,
//                               std::make_shared<MessageEvent>(*str,false)
//                               );
//                       }
//                   });
// }
