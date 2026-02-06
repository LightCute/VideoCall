//core/ClientCore.h
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
#include <atomic> // 补充 atomic 头文件，支持 std::atomic
#include "CallSession.h"
#include <optional> // 用于 optional<CallSession>

class ClientCore {
public:
    ClientCore();
    ~ClientCore();

    // 线程安全接口（对外暴露）
    void postInput(core::CoreInput ev);
    void stop();
    // 监听者管理（线程安全）
    void addListener(core::ICoreListener* listener);   // 新增
    void removeListener(core::ICoreListener* listener);// 新增

    int getMediaPort() const {
        return executor_->getMediaPort();
    }

private:
    FSM fsm_;
    State state_ = State::Disconnected;
    std::unique_ptr<CoreExecutor> executor_;

    // 线程安全队列（仅保留输入队列，输出队列废弃移除）
    std::queue<core::CoreInput>  inputQueue_;
    std::mutex mtx_;
    std::condition_variable cv_;

    // 监听者管理（新增）
    std::vector<core::ICoreListener*> listeners_;
    std::mutex listener_mtx_;  // 监听者操作的线程安全锁

    std::optional<CallSession> current_call_session_;
    // 通话相关成员变量（保留原有逻辑）
    std::string peer_;          // 通话对方用户名
    std::string peerIp_;        // 对方媒体IP
    int peerPort_ = 0;          // 对方媒体端口
    bool isCaller_ = false;     // 是否为主动呼叫方

    // 核心方法（适配新语义类型）
    void processEvents();
    void handleOutput(core::CoreOutput&& o); // 总输出处理入口
    void handleExecOutput(core::ExecOutput&& out); // 处理 Executor 输出（IO 命令）
    void broadcastUiOutput(const core::UiOutput& out); // 广播 UI 输出（状态通知）


    void endCurrentSession(const std::string& reason = "");
    // 新的 execute 重载函数（适配 ExecOutXXX 类型）
    void execute(const core::ExecOutConnect& e);
    void execute(const core::ExecOutSendLogin& e);
    void execute(const core::ExecOutSendPing& e);
    void execute(const core::ExecOutSelectLan& e);
    void execute(const core::ExecOutSelectVpn& e);
    void execute(const core::ExecOutLoginOk& e);
    void execute(const core::ExecOutSendText& e);
    void execute(const core::ExecOutSendCall& e);
    void execute(const core::ExecOutSendAcceptCall& e);
    void execute(const core::ExecOutSendRejectCall& e);
    void execute(const core::ExecOutSendMediaOffer& e);
    void execute(const core::ExecOutSendMediaAnswer& e);
    void execute(const core::ExecOutMediaReady& e);
    void execute(const core::ExecOutSendHangup& e);

    // 原子变量：控制事件处理线程运行状态
    std::atomic<bool> is_running_{true};
};
