#pragma once
#include <functional>
#include <thread>
#include <atomic>

class CommandSocket {
public:
    using AcceptCallback = std::function<void(int)>;

    bool startListen(int port);
    void setAcceptCallback(AcceptCallback cb);
    void stop();

private:
    void acceptThreadFunc();

    int listenfd_ = -1;
    std::thread acceptThread_;
    std::atomic<bool> running_{false};
    AcceptCallback acceptCb_;
};
