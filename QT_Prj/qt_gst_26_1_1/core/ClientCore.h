//ClientCore.h
#pragma once

#include <functional>
#include <string>

#include "CommandSocket.h"
#include "ClientEvent.h"

#include <variant>
#include <functional>
#include "ClientEventEnum.h"
#include "ClientState.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include "CoreOutput.h"
struct EventTableEntry {
    State current_state;
    EventType event_type;
    std::function<void(class ClientCore&, const ClientEvent)> action;
    State next_state;
};
using ActionResult = std::vector<CoreOutput>;


class ClientCore {
public:
    ClientCore();
    ~ClientCore();

    // 连接服务器
    bool doconnect(const std::string& host, int port);

    // 业务 API（UI 只调用这些）
    void sendLogin(const std::string& username, const std::string& password);
    void sendRaw(const std::string& msg); // 你现在测试用的

    // UI 注册这个回调
    std::function<void(const ClientEvent)> onEvent;
    std::function<void(State)> onStateChanged;



    void dispatchEvent(const ClientEvent ev);
    void postEvent(const ClientEvent ev);

    // UI 层可以定期或轮询调用
    bool pollEvent(ClientEvent &outEv);

private:
    State state_ = State::Disconnected;
    CommandSocket socket_;

    // 内部：socket → protocol → event
    void handleMessage(const std::string& msg);

    // 事件表
    static EventTableEntry eventTable_[];

    std::queue<ClientEvent> eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;

    // 新增一个处理队列事件的函数
    void processEvents();
};
