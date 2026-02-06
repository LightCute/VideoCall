#include "ClientCore.h"
#include <algorithm>
#include <iostream>

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
    current_call_session_.emplace();
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

// 线程安全：推入输入事件
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

// 事件处理主循环（保留原有线程模型，适配新输出逻辑）
void ClientCore::processEvents() {
    while (is_running_) {
        core::CoreInput ev;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return !inputQueue_.empty() || !is_running_; });
            if (!is_running_ && inputQueue_.empty()) break; // 退出条件：停止且队列空
            ev = std::move(inputQueue_.front());
            inputQueue_.pop();
            std::cout << "[ClientCore] Processing Input event, remaining queue size: " << inputQueue_.size()
                      << ", Event type : " << core::CoreInputIndexToName(ev.index())  << std::endl;
        }

        // 调用 FSM 处理输入，获取新语义规范的 CoreOutput 列表
        auto outputs = fsm_.handle(state_, ev);

        if (core::CoreInputIndexToName(ev.index()) == "InCallIncoming") {
            auto& incoming = std::get<core::InCallIncoming>(ev);
            // 新增：确保Session已初始化（防止空指针）
            if (!current_call_session_.has_value()) {
                current_call_session_.emplace(); // 初始化空Session
                std::cout << "[ClientCore] Create new Session for incoming call from: " << incoming.from << std::endl;
            }
            auto& session = current_call_session_.value();
            // 1. 优先写入Session（唯一真相源）
            session.peerName = incoming.from;
            session.isCaller = false;
            session.currentCallState = CallState::Ringing;
            // 2. 同步到旧字段（仅镜像，禁止直接修改旧字段）
            peer_ = incoming.from;
            isCaller_ = false;
            std::cout << "[ClientCore] Incoming call: Session updated (peer: " << session.peerName << ", sessionId: " << session.sessionId << ")" << std::endl;
        }

        {
            std::lock_guard<std::mutex> lock(mtx_);
            for (auto& o : outputs) {
                handleOutput(std::move(o)); // 处理每个输出事件
            }
        }
    }
}

// core/ClientCore.cpp
void ClientCore::endCurrentSession(const std::string& reason) {
    // 1. 校验：当前是否有活跃会话
    if (!current_call_session_.has_value()) {
        std::cerr << "[ClientCore] No active session to end" << std::endl;
        return;
    }
    auto& session = current_call_session_.value();
    std::cout << "[ClientCore] Ending session: " << session.sessionId << ", reason: " << reason << std::endl;

    // 2. 重置 Session（唯一真相源）
    current_call_session_.reset();

    // 3. 同步旧字段（清空）
    peer_.clear();
    peerIp_.clear();
    peerPort_ = 0;
    isCaller_ = false;

    // 4. 收口操作：停止媒体、发送挂断信令（复用原有逻辑）
    executor_->sendHangup(); // 原有挂断信令逻辑
    // 触发 UI 停止媒体（复用原有 UiOutStopMedia）
    broadcastUiOutput(core::UiOutput{core::UiOutStopMedia{}});

    // 5. 同步旧 State 到 LoggedIn（复用原有逻辑）
    core::UiOutStateChanged uiStateChange{state_, State::LoggedIn};
    broadcastUiOutput(core::UiOutput{uiStateChange});
    state_ = State::LoggedIn;

    std::cout << "[ClientCore] Session ended successfully: " << session.sessionId << std::endl;
}

// 核心：总输出处理入口，实现三路分流
void ClientCore::handleOutput(core::CoreOutput&& o) {
    std::visit([this](auto&& out){
        using T = std::decay_t<decltype(out)>;

        // 分流1：处理 Executor 输出（IO 命令）
        if constexpr (std::is_same_v<T, core::ExecOutput>) {
            handleExecOutput(std::move(out));
        }
        // 分流2：广播 UI 输出（状态通知）
        else if constexpr (std::is_same_v<T, core::UiOutput>) {
            // 先处理状态变更，更新内部 state_
            std::visit([this](auto&& ui_out){
                using UiT = std::decay_t<decltype(ui_out)>;
                if constexpr (std::is_same_v<UiT, core::UiOutStateChanged>) {
                    std::cout << "[Core] State: "
                              << stateToString(ui_out.from)
                              << " -> "
                              << stateToString(ui_out.to)
                              << std::endl;
                    state_ = ui_out.to; // 更新内部状态
                }
            }, out);
            // 广播 UI 事件给所有监听者
            broadcastUiOutput(out);
        }
        // 分流3：处理 Core 内部辅助事件（无对外交互）
        else if constexpr (std::is_same_v<T, core::InternalOutUpdateAlive>) {
            std::cout << "[Executor] Received PONG from server" << std::endl;
        }
    }, std::move(o));
}

// 分流函数1：处理 Executor 输出，分发到对应 execute 方法
void ClientCore::handleExecOutput(core::ExecOutput&& out) {
    std::visit([this](auto&& e){
        using T = std::decay_t<decltype(e)>;

        // 按 ExecOutXXX 类型分发（覆盖所有 CoreOutput.h 定义的类型）
        if constexpr (std::is_same_v<T, core::ExecOutConnect>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendLogin>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendPing>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSelectLan>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSelectVpn>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutLoginOk>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendText>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendCall>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendAcceptCall>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendRejectCall>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendMediaOffer>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendMediaAnswer>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutMediaReady>) {
            execute(std::move(e));
        }
        else if constexpr (std::is_same_v<T, core::ExecOutSendHangup>) {
            //execute(std::move(e));
            this->endCurrentSession("user hangup");
        }
    }, std::move(out));
}

// 分流函数2：广播 UI 输出，仅发送 UiOutput 给监听者（线程安全）
void ClientCore::broadcastUiOutput(const core::UiOutput& out) {
    std::lock_guard<std::mutex> lock(listener_mtx_);
    for (auto* l : listeners_) {
        if (l) { // 防止空指针
            l->onUiOutput(out); // 调用监听者的 UI 回调
        }
    }
}

// ========== 新的 execute 重载函数（适配 ExecOutXXX 类型） ==========
void ClientCore::execute(const core::ExecOutConnect& e) {
    executor_->connectToServer(e.host, e.port);
}

void ClientCore::execute(const core::ExecOutSendLogin& e) {
    executor_->sendLoginRequest(e.user, e.pass);
}

void ClientCore::execute(const core::ExecOutSendPing& e) {
    (void)e; // 消除未使用参数警告
    executor_->sendPing();
}

void ClientCore::execute(const core::ExecOutSelectLan& e) {
    (void)e; // 消除未使用参数警告
    std::cout << "[Executor] Select LAN" << std::endl;
    executor_->setLanMode();
}

void ClientCore::execute(const core::ExecOutSelectVpn& e) {
    (void)e; // 消除未使用参数警告
    std::cout << "[Executor] Select VPN" << std::endl;
    executor_->setVpnMode();
}

void ClientCore::execute(const core::ExecOutLoginOk& e) {
    (void)e; // 消除未使用参数警告
    std::cout << "[Executor] ExecOutLoginOk: Send local IP to server" << std::endl;
    executor_->sendLocalIP();
}

void ClientCore::execute(const core::ExecOutSendText& e) {
    executor_->sendTextMsg(e.target_user, e.content);
}

void ClientCore::execute(const core::ExecOutSendCall& e) {
    // 第一步：先写入 CallSession（唯一真相源）
    if (current_call_session_.has_value()) {
        auto& session = current_call_session_.value();
        session.peerName = e.target_user; // 写入 peerName
        session.isCaller = true;          // 写入 isCaller
        session.currentCallState = CallState::Calling; // 同步状态
    }

    // 第二步：同步到旧字段（保留原有逻辑，不删除）
    isCaller_ = true;
    peer_ = e.target_user; // 补充：原有代码未写 peer_，这里补上以同步

    // 原有逻辑：调用 executor 发送呼叫
    executor_->sendCallRequest(e.target_user);
}

void ClientCore::execute(const core::ExecOutSendAcceptCall& e) {
    (void)e;
    executor_->sendAcceptCall();

    // 新增：更新Session状态（唯一真相源）
    if (current_call_session_.has_value()) {
        auto& session = current_call_session_.value();
        session.currentCallState = CallState::InCall; // 标记为“通话中”
        std::cout << "[ClientCore] Accept call: Session state updated to InCall (sessionId: " << session.sessionId << ")" << std::endl;
    }
}

void ClientCore::execute(const core::ExecOutSendRejectCall& e) {
    (void)e;
    executor_->sendRejectCall();

    // 新增：拒绝通话直接结束Session（统一走endCurrentSession）
    endCurrentSession("user reject call");
    std::cout << "[ClientCore] Reject call: Session ended" << std::endl;
}

void ClientCore::execute(const core::ExecOutSendMediaOffer& e) {
    executor_->sendMediaOffer(e.peer);
}

void ClientCore::execute(const core::ExecOutSendMediaAnswer& e) {
    executor_->sendMediaAnswer(e.peer);
}

void ClientCore::execute(const core::ExecOutSendHangup& e) {
    (void)e; // 消除未使用参数警告
    executor_->sendHangup();
}

void ClientCore::execute(const core::ExecOutMediaReady& e) {
    // 1. 计算最终Peer IP/Port（仅计算，不直接写入旧字段）
    std::string peerIp = executor_->selectPeerIp(e.lanIp, e.vpnIp);
    int peerPort = e.peerPort;

    // 2. 优先写入Session（唯一真相源）
    if (current_call_session_.has_value()) {
        auto& session = current_call_session_.value();
        session.peerIp = peerIp;
        session.peerPort = peerPort;
        session.currentCallState = CallState::MediaReady;
        std::cout << "[ClientCore] MediaReady: Session updated (peerIp: " << peerIp << ", port: " << peerPort << ")" << std::endl;
    }

    // 3. 同步到旧字段（仅镜像，禁止直接修改）
    peerIp_ = peerIp;
    peerPort_ = peerPort;

    // 4. 原有逻辑：构建UI输出（数据来自Session的镜像）
    std::cout << "[ClientCore] MediaReady -> use IP: " << peerIp_ << ":" << peerPort_ << std::endl;
    core::UiOutMediaReadyFinal uiMediaReadyFinal{peerIp_, peerPort_};
    broadcastUiOutput(core::UiOutput{uiMediaReadyFinal});
}
