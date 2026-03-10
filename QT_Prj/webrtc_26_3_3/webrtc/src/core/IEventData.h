#pragma once
enum class EventType {
    UI_NetConnectServer,
    Net_RecvMessage,
    UI_SendMessage
};

class IEventData {
public:
    virtual ~IEventData() = default;
};
