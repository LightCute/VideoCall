// core/ClientCore.h
#pragma once
#include "CoreInput.h"
#include "CoreOutput.h"
#include "FSM.h"
#include "CommandSocket.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "protocol_text.h"
#include <iostream>

class ClientCore {
public:
    ClientCore();
    ~ClientCore();

    // 线程安全接口
    void postInput(core::CoreInput ev);
    bool pollOutput(CoreOutput& out);

    // 可选接口
    bool connectToServer(const std::string& host, int port);
    void sendLogin(const std::string& user, const std::string& pass);

private:
    FSM fsm_;
    CommandSocket socket_;

    std::queue<core::CoreInput>  inputQueue_;
    std::queue<CoreOutput> outputQueue_;
    std::mutex mtx_;
    std::condition_variable cv_;

    void processEvents(); // 后台线程
};
