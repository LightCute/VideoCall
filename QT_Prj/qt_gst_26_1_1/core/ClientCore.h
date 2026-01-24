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
#include "ICoreListener.h"
#include <vector>

class ClientCore {
public:
    ClientCore();
    ~ClientCore();

    // 线程安全接口（对外暴露）
    void postInput(core::CoreInput ev);
    // bool pollOutput(core::CoreOutput& out); // 补充 core:: 前缀
    void stop();
    // 监听者管理（线程安全）
    void addListener(core::ICoreListener* listener);   // 新增
    void removeListener(core::ICoreListener* listener);// 新增

private:
    FSM fsm_;
    State state_ = State::Disconnected;
    std::unique_ptr<CoreExecutor> executor_;

    // 线程安全队列（补充 core:: 前缀）
    std::queue<core::CoreInput>  inputQueue_;
    std::queue<core::CoreOutput> outputQueue_; // 补充 core:: 前缀
    std::mutex mtx_;
    std::condition_variable cv_;

    // 监听者管理（新增）
    std::vector<core::ICoreListener*> listeners_;
    std::mutex listener_mtx_;  // 监听者操作的线程安全锁

    std::string peer_;          // 通话对方用户名
    std::string peerIp_;        // 对方媒体IP
    int peerPort_ = 0;          // 对方媒体端口
    bool isCaller_ = false;     // 是否为主动呼叫方
    // 核心方法（参数补充 core:: 前缀）
    void processEvents();
    void applyStateChange(const core::OutStateChanged& e); // 补充 core:: 前缀
    void handleOutput(core::CoreOutput&& o); // 补充 core:: 前缀
    void execute(const core::OutConnect& e) ; // 补充 core:: 前缀
    void execute(const core::OutSendLogin& e) ; // 补充 core:: 前缀
    void execute(const core::OutSendPing& e);
    void execute(const core::OutUpdateAlive&);
    void execute(const core::OutSelectLan&);
    void execute(const core::OutSelectVpn&) ;
    void execute(const core::OutLoginOk&);
    void execute(const core::OutSendText& e);
    void execute(const core::OutForwardText&);
    void execute(const core::OutSendCall& e);
    void execute(const core::OutSendAcceptCall& e);
    void execute(const core::OutSendRejectCall& e);
    void execute(const core::OutSendMediaOffer& e);
    void execute(const core::OutSendMediaAnswer& e);
    void execute(const core::OutShowIncomingCall& e);
    void execute(const core::OutMediaReady& e);
    void broadcastOutput(const core::CoreOutput& out);
    std::atomic<bool> is_running_{true};
};
