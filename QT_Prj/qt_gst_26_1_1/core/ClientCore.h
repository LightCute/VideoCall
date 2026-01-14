#pragma once
#include "CoreInput.h"
#include "CoreOutput.h"
#include "FSM.h"
#include "ClientState.h"
#include "CoreExecutor.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>

class ClientCore {
public:
    ClientCore();
    ~ClientCore();

    // 线程安全接口（对外暴露）
    void postInput(core::CoreInput ev);
    bool pollOutput(core::CoreOutput& out); // 补充 core:: 前缀

    // 废弃：原直接操作 socket 的接口
    // bool connectToServer(const std::string& host, int port);
    // void sendLogin(const std::string& user, const std::string& pass);

private:
    FSM fsm_;
    State state_ = State::Disconnected;
    std::unique_ptr<CoreExecutor> executor_;

    // 线程安全队列（补充 core:: 前缀）
    std::queue<core::CoreInput>  inputQueue_;
    std::queue<core::CoreOutput> outputQueue_; // 补充 core:: 前缀
    std::mutex mtx_;
    std::condition_variable cv_;

    // 核心方法（参数补充 core:: 前缀）
    void processEvents();
    void applyStateChange(const core::OutStateChanged& e); // 补充 core:: 前缀
    void handleOutput(core::CoreOutput&& o); // 补充 core:: 前缀
    void execute(const core::OutConnect& e) ; // 补充 core:: 前缀
    void execute(const core::OutSendLogin& e) ; // 补充 core:: 前缀
};
