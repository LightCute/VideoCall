//ClientCore.cpp
#include "ClientCore.h"

#include "ClientEventFactory.h"
#include "protocol_text.h"   // makeLoginRequest
#include <QDebug>

ClientCore::ClientCore() {
    // 让 CommandSocket 只认识 ClientCore，不认识 UI
    socket_.setMessageCallback([this](const std::string& msg) {
        handleMessage(msg);
    });

    socket_.setConnectCallback([this]{
        dispatchEvent(EvTcpConnected{});
    });

    socket_.setDisconnectCallback([this]{
        dispatchEvent(EvTcpDisconnected{});
    });

    std::thread([this] { processEvents(); }).detach();
}

ClientCore::~ClientCore() {
    socket_.stop();
}


EventTableEntry ClientCore::eventTable_[] = {

    // ===== 断开态 =====
    { State::Disconnected, EventType::CmdConnect,
        [](ClientCore& c, const ClientEvent& ev){
            if (auto e = std::get_if<EvCmdConnect>(&ev)) {
             c.state_ = State::Connecting;
                c.doconnect(e->host, e->port);
            }
        },
        State::Connecting
    },

    // ===== 正在连接 =====
    { State::Connecting, EventType::TcpConnected,
        [](ClientCore& c, const ClientEvent& ev){
            if (auto e = std::get_if<EvTcpConnected>(&ev)) {
                if (c.onEvent) c.onEvent(std::move(*e)); // 移动给 UI

            }
        },
        State::Connected
    },

    { State::Connecting, EventType::TcpDisconnected,
        nullptr,
        State::Disconnected
    },

    // ===== 已连接 =====
    { State::Connected, EventType::CmdLogin,
        [](ClientCore& c, const ClientEvent& ev){
            if (auto e = std::get_if<EvCmdLogin>(&ev)) {
                c.sendLogin(e->user, e->pass);
            }
        },
        State::LoggingIn
    },

    // ===== 正在登录 =====
    { State::LoggingIn, EventType::LoginOk,
        [](ClientCore& c, const ClientEvent& ev){
            if (auto e = std::get_if<EvLoginOk>(&ev)) {
                if (c.onEvent) c.onEvent(std::move(*e)); // 移动给 UI
            }
        },
        State::LoggedIn
    },

    { State::LoggingIn, EventType::LoginFail,
        [](ClientCore& c, const ClientEvent& ev){
            if (auto e = std::get_if<EvLoginFail>(&ev)) {
                if (c.onEvent) c.onEvent(std::move(*e)); // 移动给 UI
            }
        },
        State::Connected
    },

    // ===== 任何在线状态断线 =====
    { State::Connected,  EventType::TcpDisconnected, nullptr, State::Disconnected },
    { State::LoggingIn,  EventType::TcpDisconnected, nullptr, State::Disconnected },
    { State::LoggedIn,   EventType::TcpDisconnected, nullptr, State::Disconnected },

};



void ClientCore::dispatchEvent(const ClientEvent ev)
{
    qDebug() << "dispatchEvent called, state=" << (int)state_;
    std::visit([](auto&& e){
        qDebug() << "Event type:" << typeid(e).name();
    }, ev);

    EventType evType;

    // 根据 ClientEvent 类型确定 EventType
    std::visit([&evType](auto&& e){
        using T = std::decay_t<decltype(e)>;

        // ===== UI =====
        if constexpr (std::is_same_v<T, EvCmdConnect>)      evType = EventType::CmdConnect;
        else if constexpr (std::is_same_v<T, EvCmdDisconnect>) evType = EventType::CmdDisconnect;
        else if constexpr (std::is_same_v<T, EvCmdLogin>)   evType = EventType::CmdLogin;

        // ===== TCP =====
        else if constexpr (std::is_same_v<T, EvTcpConnected>)    evType = EventType::TcpConnected;
        else if constexpr (std::is_same_v<T, EvTcpDisconnected>) evType = EventType::TcpDisconnected;

        // ===== 协议 =====
        else if constexpr (std::is_same_v<T, EvLoginOk>)   evType = EventType::LoginOk;
        else if constexpr (std::is_same_v<T, EvLoginFail>) evType = EventType::LoginFail;
        else if constexpr (std::is_same_v<T, EvOnlineUsers>)     evType = EventType::OnlineUsers;

        else evType = EventType::Unknow;
    }, ev);


    bool matched = false;
    // 遍历事件表匹配
    for (const auto& entry : eventTable_) {
        qDebug() << "FSM entry: state=" << (int)entry.current_state
                 << " event_type=" << (int)entry.event_type
                 << " current state_=" << (int)state_
                 << " evType=" << (int)evType;
        if (entry.current_state == state_ && entry.event_type == evType) {
            qDebug() << "Matched, executing action";
            // ① 先变状态
            state_ = entry.next_state;

            // ② 先通知 UI
            if (onStateChanged)
                onStateChanged(state_);

            // ③ 再执行 action（里面可能会触发新事件）
            if (entry.action)
                entry.action(*this, std::move(ev));
            matched = true;
            break;
        }
    }
    if(!matched) {
        qDebug() << "No matching event table entry for state" << (int)state_ << "event" << (int)evType;
    }
}



bool ClientCore::doconnect(const std::string& host, int port) {
    return socket_.connectToServer(host, port);
}

void ClientCore::sendLogin(const std::string& username,
                           const std::string& password)
{
    std::string msg = proto::makeLoginRequest(username, password);
    socket_.sendMessage(msg);
}


// void ClientCore::postEvent(const ClientEvent ev)
// {
//     dispatchEvent(std::move(ev));
// }
void ClientCore::postEvent(ClientEvent ev)  // 注意按值 + 移动
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        eventQueue_.push(std::move(ev));
    }
    queueCv_.notify_one(); // 通知可能在等待的线程
}

void ClientCore::processEvents()
{
    while (true) {
        ClientEvent ev;

        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this]{ return !eventQueue_.empty(); });

            ev = std::move(eventQueue_.front());
            eventQueue_.pop();
        }

        dispatchEvent(std::move(ev));
    }
}


void ClientCore::sendRaw(const std::string& msg) {
    socket_.sendMessage(msg);
}

// ========================
// 这是客户端的大脑
// ========================
void ClientCore::handleMessage(const std::string& msg) {
    // 1️⃣ 协议解析
    ClientEvent ev = ClientEventFactory::makeEvent(msg);

    // 2️⃣ 投递给 UI（或者上层）
    dispatchEvent(std::move(ev));
}


bool ClientCore::pollEvent(ClientEvent &outEv) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if(eventQueue_.empty()) return false;
    outEv = std::move(eventQueue_.front());
    eventQueue_.pop();
    return true;
}


