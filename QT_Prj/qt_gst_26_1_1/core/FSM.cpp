// core/FSM.cpp
#include "FSM.h"

FSM::FSM(State init) : state_(init) {
    // 新增：构造函数中初始化状态表
    initTable();
}

std::vector<CoreOutput> FSM::handle(core::CoreInput ev) {
    EventType evType = eventTypeFromInput(ev);
    std::cout << "[FSM] event type: " << static_cast<int>(evType) << " current state: " << static_cast<int>(state_) << std::endl;
    for (auto& entry : table_) {
        if (entry.current_state == state_ && entry.event_type == evType) {
            // 事件类型匹配（简化示例，用 type_index 或自定义 EventType）
            // 如果匹配：
            auto outputs = entry.action(ev);
            std::cout << "[FSM] matched，output num: " << outputs.size() << std::endl;
            state_ = entry.next_state;
            return outputs;
        }
    }
    std::cout << "[FSM] no matched" << std::endl;
    return {};
}

EventType FSM::eventTypeFromInput(const core::CoreInput& ev) {
    EventType evType = EventType::Unknow;
    std::visit([&evType](auto&& e){
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, core::EvCmdConnect>)      evType = EventType::CmdConnect;
        else if constexpr (std::is_same_v<T, core::EvCmdDisconnect>) evType = EventType::CmdDisconnect;
        else if constexpr (std::is_same_v<T, core::EvCmdLogin>)   evType = EventType::CmdLogin;

        else if constexpr (std::is_same_v<T, core::EvTcpConnected>)    evType = EventType::TcpConnected;
        else if constexpr (std::is_same_v<T, core::EvTcpDisconnected>) evType = EventType::TcpDisconnected;

        else if constexpr (std::is_same_v<T, core::EvLoginOk>)   evType = EventType::LoginOk;
        else if constexpr (std::is_same_v<T, core::EvLoginFail>) evType = EventType::LoginFail;
        else if constexpr (std::is_same_v<T, core::EvOnlineUsers>) evType = EventType::OnlineUsers;

        else evType = EventType::Unknow;
    }, ev);
    return evType;
}


void FSM::initTable() {
    table_ = {
      // ===== 断开态 =====
        { State::Disconnected, EventType::CmdConnect,
          [](const core::CoreInput& ev) -> std::vector<CoreOutput> {
              std::vector<CoreOutput> out;
              if (auto e = std::get_if<core::EvCmdConnect>(&ev)) {
                  // 在 FSM 里只能返回输出，不直接操作 Core
                  // 可以用输出通知 Core 做连接
                  out.push_back(OutStateChanged{State::Connecting});
                  out.push_back(OutConnect{e->host,e->port});
                  // 可用特殊输出通知 Core 去 connect
                  // 也可以让 ClientCore 在 postInput 后立即 connect
              }
              return out;
          },
          State::Connecting
      },

      // ===== 正在连接 =====
        { State::Connecting, EventType::TcpConnected,
          [](const core::CoreInput& ev) -> std::vector<CoreOutput> {
              std::vector<CoreOutput> out;
             if (auto e = std::get_if<core::EvTcpConnected>(&ev)) {
                  out.push_back(OutStateChanged{State::Connected});
              }
              return out;
          },
          State::Connected
      },

        { State::Connecting, EventType::TcpDisconnected,
          [](const core::CoreInput&){ return std::vector<CoreOutput>{}; },
          State::Disconnected
      },

      // ===== 已连接 =====
        { State::Connected, EventType::CmdLogin,
          [](const core::CoreInput& ev) -> std::vector<CoreOutput> {
              std::vector<CoreOutput> out;
              if (auto e = std::get_if<core::EvCmdLogin>(&ev)) {
                  // Core 会在 ClientCore 层处理 sendLogin
                  out.push_back(OutStateChanged{State::LoggingIn});
                  out.push_back(OutSendLogin{e->user, e->pass});
              }
              return out;
          },
          State::LoggingIn
      },

      // ===== 正在登录 =====
        { State::LoggingIn, EventType::LoginOk ,
         [](const core::CoreInput& ev) -> std::vector<CoreOutput> {
              std::vector<CoreOutput> out;
             if (auto e = std::get_if<core::EvLoginOk>(&ev)) {
                  out.push_back(OutLoginOk{});
                  out.push_back(OutStateChanged{State::LoggedIn});
              }
              return out;
          },
          State::LoggedIn
      },

        { State::LoggingIn, EventType::LoginFail ,
          [](const core::CoreInput& ev) -> std::vector<CoreOutput> {
              std::vector<CoreOutput> out;
              if (auto e = std::get_if<core::EvLoginFail>(&ev)) {
                  out.push_back(OutLoginFail{e->msg});
                  out.push_back(OutStateChanged{State::Connected});
              }
              return out;
          },
          State::Connected
      },

      // ===== 任何在线状态断线 =====
      { State::Connected,  EventType::TcpDisconnected,
          [](const core::CoreInput&){ return std::vector<CoreOutput>{OutStateChanged{State::Disconnected}, OutDisconnected{}}; },
          State::Disconnected
      },
      { State::LoggingIn,  EventType::TcpDisconnected,
          [](const core::CoreInput&){ return std::vector<CoreOutput>{OutStateChanged{State::Disconnected}, OutDisconnected{}}; },
          State::Disconnected
      },
      { State::LoggedIn,   EventType::TcpDisconnected,
          [](const core::CoreInput&){ return std::vector<CoreOutput>{OutStateChanged{State::Disconnected}, OutDisconnected{}}; },
          State::Disconnected
      },
  };
}


