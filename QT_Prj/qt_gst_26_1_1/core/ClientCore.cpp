// core/ClientCore.cpp
#include "ClientCore.h"


ClientCore::ClientCore() : fsm_(State::Disconnected) {
    socket_.setMessageCallback([this](const std::string& msg){
        // 转成 CoreInput
        postInput(core::EvLoginOk{}); // 示例
    });

    socket_.setConnectCallback([this]{
        postInput(core::EvTcpConnected{});
    });

    socket_.setDisconnectCallback([this]{
        postInput(core::EvTcpDisconnected{});
    });

    std::thread([this]{ processEvents(); }).detach();
}

ClientCore::~ClientCore() {
    socket_.stop();
}

void ClientCore::postInput(core::CoreInput ev) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        inputQueue_.push(std::move(ev));
    }
    cv_.notify_one();
}

bool ClientCore::pollOutput(CoreOutput& out) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (outputQueue_.empty()) return false;
    out = std::move(outputQueue_.front());
    outputQueue_.pop();
    return true;
}

void ClientCore::processEvents() {
    while (true) {
        core::CoreInput ev;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this]{ return !inputQueue_.empty(); });
            ev = std::move(inputQueue_.front());
            inputQueue_.pop();
        }

        auto outputs = fsm_.handle(std::move(ev));
        {
            std::lock_guard<std::mutex> lock(mtx_);
            for (auto& o : outputs) {
                // ===== 修正后的 OutConnect 处理逻辑 =====
                if (auto connectEv = std::get_if<OutConnect>(&o)) {
                    // 临时变量拷贝（兼容C++11）
                    std::string host = connectEv->host;
                    int port = connectEv->port;

                    // 异步执行连接
                    std::thread connectThread([this, host, port]() {
                        bool connectResult = this->socket_.connectToServer(host, port);
                        if (!connectResult) {
                            this->postInput(core::EvTcpDisconnected{});
                        }
                    });
                    connectThread.detach();
                }
                // ===== OutSendLogin 处理逻辑（原有）=====
                if (auto loginEv = std::get_if<OutSendLogin>(&o)) {
                    std::string loginMsg = proto::makeLoginRequest(loginEv->user, loginEv->pass);
                    socket_.sendMessage(loginMsg);
                }

                // 🔴 关键：先把 OutStateChanged 写入队列，再处理其他逻辑
                std::cout << "[ClientCore] write outputQueue_, type index: " << o.index() << std::endl;
                outputQueue_.push(std::move(o));
            }
        }
    }
}
bool ClientCore::connectToServer(const std::string& host, int port) {
    return socket_.connectToServer(host, port);
}

void ClientCore::sendLogin(const std::string& user, const std::string& pass) {
    socket_.sendMessage("login"); // 伪示例
}
