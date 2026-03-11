#include "Websocket.h"
#include <iostream>
#include "../core/ConnectEvent.h"
#include "../utilities/log.h"

WebSocketClient::WebSocketClient(IEventQueue* queue)
    : m_queue( queue)
{
    Log::info("[NET] WebSocketClient constructed");

    m_queue->appendListener(
        EventType::UI_NetConnectServer,
        [this](std::shared_ptr<IEventData> data)
        {
            Log::debug("[NET] received UI_NetConnectServer event");
            auto msg = std::dynamic_pointer_cast<MessageEvent>(data);
            if(msg)
            {                
                Log::info("[NET] connecting to {}", msg->getURL());
                connect(msg->getURL());
            }                
        }
    );
}

WebSocketClient::~WebSocketClient()
{
    close();
}

void WebSocketClient::connect(const std::string& url)
{
    Log::info("[NET] connect {}", url);
    running = true;

    networkThread = std::thread([this, url]()
    {
        Log::info("[NET] network thread started");
        ws = std::make_shared<rtc::WebSocket>();
        Log::debug("[NET] rtc::WebSocket created");
        ws->onOpen([]()
        {
            Log::info("[NET] WebSocket onOpen");
        });

        ws->onClosed([]()
        {
            Log::info("[NET] WebSocket onClosed");
        });

        ws->onMessage([this](rtc::message_variant msg)
        {
            Log::debug("[NET] WebSocket onMessage");

            if(auto str = std::get_if<std::string>(&msg))
            {
                Log::info("[NET] received message {}", *str);
            }
        });
        Log::info("[NET] opening websocket {}", url);
        ws->open(url);

        while(running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        Log::info("[NET] network thread exiting");
    });
}

void WebSocketClient::send(const std::string& msg)
{
    Log::debug("[NET] send {}", msg);
    if(ws)
        ws->send(msg);
}

void WebSocketClient::close()
{
    Log::info("[NET] closing websocket client");
    running = false;

    if(ws)
    {
        Log::debug("[NET] closing websocket");
        ws->close();
    }

    if(networkThread.joinable())
    {
        Log::debug("[NET] joining network thread");
        networkThread.join();
    }

    Log::info("[NET] websocket client closed");
}



