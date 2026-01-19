#include "ClientCore.h"
#include <algorithm>

// 构造函数：初始化 Executor，设置回调
ClientCore::ClientCore() : fsm_() {
    // 初始化 Executor：回调函数用于接收 Executor 推送的 CoreInput
    executor_ = std::make_unique<CoreExecutor>(
        [this](core::CoreInput ev) {
            this->postInput(std::move(ev)); // 将 Executor 的事件推入输入队列
        }
        );

    // 启动事件处理线程（仅调度，无 IO）
    std::thread([this]{ processEvents(); }).detach();
}

ClientCore::~ClientCore() {
    // 释放 Executor 资源
    stop();
    if (executor_) {
        executor_->stop();
    }
}

void ClientCore::stop() {
    is_running_ = false;
    cv_.notify_one(); // 唤醒等待的线程，使其退出循环
}
// 原 postInput/pollOutput 逻辑不变（补充 core:: 前缀）
void ClientCore::postInput(core::CoreInput ev) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        inputQueue_.push(std::move(ev));
    }
    cv_.notify_one();
}

// 新增：添加监听者（线程安全）
void ClientCore::addListener(core::ICoreListener* listener) {
    std::lock_guard<std::mutex> lock(listener_mtx_);
    listeners_.push_back(listener);
}

// 新增：移除监听者（线程安全）
void ClientCore::removeListener(core::ICoreListener* listener) {
    std::lock_guard<std::mutex> lock(listener_mtx_);
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end()
        );
}

// 新增：广播输出事件给所有监听者（Core 线程调用）
void ClientCore::broadcastOutput(const core::CoreOutput& out) {
    std::lock_guard<std::mutex> lock(listener_mtx_);
    for (auto* listener : listeners_) {
        listener->onCoreOutput(out); // 调用监听者的回调（Core 线程）
    }
}


// bool ClientCore::pollOutput(core::CoreOutput& out) { // 补充 core:: 前缀
//     std::lock_guard<std::mutex> lock(mtx_);
//     if (outputQueue_.empty()) return false;
//     out = std::move(outputQueue_.front());
//     outputQueue_.pop();
//     return true;
// }

// 原 processEvents 逻辑不变（补充 core:: 前缀）
void ClientCore::processEvents() {
    while (is_running_) {
        core::CoreInput ev;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return !inputQueue_.empty(); });
            ev = std::move(inputQueue_.front());
            inputQueue_.pop();
            std::cout << "[ClientCore] Processing Input event, remaining queue size: " << inputQueue_.size()
                      << ", Event type index: " << ev.index() << std::endl;
        }

        auto outputs = fsm_.handle(state_, ev);
        {
            std::lock_guard<std::mutex> lock(mtx_);
            for (auto& o : outputs) {
                handleOutput(std::move(o));
            }
        }
    }
}

// 原 handleOutput/applyStateChange 逻辑不变（补充 core:: 前缀）
void ClientCore::handleOutput(core::CoreOutput&& o) { // 补充 core:: 前缀
    std::visit([this](auto&& e){
        using T = std::decay_t<decltype(e)>;

        if constexpr (std::is_same_v<T, core::OutStateChanged>) { // 补充 core:: 前缀
            applyStateChange(e);
            broadcastOutput(e); // Broadcast state change
        }
        else if constexpr (std::is_same_v<T, core::OutConnect>) { // 补充 core:: 前缀
            execute(e);
        }
        else if constexpr (std::is_same_v<T, core::OutSendLogin>) { // 补充 core:: 前缀
            execute(e);
        }
        else if constexpr (std::is_same_v<T, core::OutSendPing>) {
            execute(e);
        }
        else if constexpr (std::is_same_v<T, core::OutUpdateAlive>) {
            execute(e);
            broadcastOutput(e);
        }
        else {
            // outputQueue_.push(std::move(e));
            broadcastOutput(e); // 广播其他事件
        }
    }, std::move(o));
}

void ClientCore::applyStateChange(const core::OutStateChanged& e) { // 补充 core:: 前缀
    std::cout << "[Core] State: "
              << stateToString(e.from)
              << " -> "
              << stateToString(e.to)
              << std::endl;

    state_ = e.to;
    //outputQueue_.push(e);
}

// 关键修改：execute 仅调用 Executor 接口，无 IO 逻辑（补充 core:: 前缀）
void ClientCore::execute(const core::OutConnect& e) { // 补充 core:: 前缀
    // 调度 Executor 执行连接（Core 仅发命令，不做具体操作）
    executor_->connectToServer(e.host, e.port);
}

void ClientCore::execute(const core::OutSendLogin& e) { // 补充 core:: 前缀
    // 调度 Executor 发送登录请求
    executor_->sendLoginRequest(e.user, e.pass);
}

void ClientCore::execute(const core::OutSendPing&) {
    executor_->sendPing();
}

void ClientCore::execute(const core::OutUpdateAlive&) {
    std::cout << "[Executor] Received PONG from server" << std::endl;
}


// 废弃原 socket 操作接口（可直接删除）
// bool ClientCore::connectToServer(const std::string& host, int port) { ... }
// void ClientCore::sendLogin(const std::string& user, const std::string& pass) { ... }
