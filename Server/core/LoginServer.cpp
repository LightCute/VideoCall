//LoginServer.cpp
#include "./core/LoginServer.h"


using namespace proto;

// 实现 LoginServer 构造函数
LoginServer::LoginServer() 
    : dispatcher_(loginService_, sessionMgr_)  // 手动初始化 dispatcher_，传入所需两个参数
{
    // 构造函数体可留空（如需其他初始化逻辑可补充）
}

bool LoginServer::start(int port) {
    listener_.setAcceptCallback(
        [this](int clientfd) {
            onAccept(clientfd);
        }
    );
    return listener_.startListen(port);
}



void LoginServer::onAccept(int clientfd) {
    std::cout << "[Server] new client fd=" << clientfd << std::endl;

    pool_.post([this, clientfd] {
        clientThread(clientfd);
    });
}

void LoginServer::clientThread(int clientfd) {
    std::vector<char> recvBuffer;
    char buf[1024];

    while (true) {
        int n = recv(clientfd, buf, sizeof(buf), 0);
        if (n <= 0) break;

        recvBuffer.insert(recvBuffer.end(), buf, buf + n);

        std::string payload;
        while (PacketCodec::tryDecode(recvBuffer, payload)) {
            onMessage(clientfd, payload);
        }
    }



    close(clientfd);
    std::cout << "[Server] client disconnected fd=" << clientfd << std::endl;
}

void LoginServer::onMessage(int clientfd, const std::string& msg) {
    std::cout << "[Server] recv from " << clientfd << ": " << msg << std::endl;

    ServerEvent event = ServerEventFactory::makeEvent(msg);

    ServerAction action = dispatcher_.dispatch(clientfd, event);
    
    handleAction(action);

    
}

void LoginServer::handleAction(const ServerAction& action) {
    switch (action.type) {
    case ServerActionType::SendLoginResult: {
        const auto& r = action.loginResult;

        std::string payload;

        if (r.success) {
            proto::UserInfo u;
            u.username = r.username;
            u.privilege = r.privilege;

            payload = proto::makeLoginOk(u, "welcome");
        } else {
            payload = proto::makeLoginFail(r.reason);
        }

        listener_.sendPacket(action.targetFd, payload);
        break;
    }

    default:
        break;
    }
}



// void LoginServer::handleLogin(int fd, ServerEvent& event)
// {
//     if (event.type != ServerEventType::LoginRequest)
//         return;

//    switch (event.type) {
//     case ServerEventType::LoginRequest:

//         break;

//     default:

//         break;
//     }


//     //auto result = loginService_.handleLogin(event.loginReq);

//     // if (result.success) {
//     //     sessionMgr_.login(fd, result.user);
//     //     sendPacket(fd, proto::makeLoginOk(...));
//     // } else {
//     //     sendPacket(fd, proto::makeLoginFail("fail"));
//     // }
// }



// void LoginServer::handleLogin(int fd, ServerEvent& event)
// {
//    switch (event.type) {
//     case ServerEventType::LoginRequest:
        
//     // std::string user, pwd;
//     // if (proto::parseLoginRequest(msg, user, pwd) == true) {
//     //     proto::UserInfo user_temp;

//     //     if(user == "admin" && pwd == "123")
//     //     {
//     //         std::cout << "Login suceess admin 123" << std::endl;
//     //         user_temp.username = "admin";
//     //         user_temp.privilege = 10;

//     //         {
//     //             std::lock_guard<std::mutex> lock(mutex_);
//     //             clients_[clientfd] = {
//     //                 user,
//     //                 user_temp.privilege,
//     //                 "unknown",
//     //                 0,
//     //                 true
//     //             };
//     //         }

//     //         sendPacket(clientfd, proto::makeLoginOk(user_temp, "welcome"));

//     //         broadcastOnlineUsers();
//     //     }
//     //     else {
//     //         sendPacket(clientfd, proto::makeLoginFail("Login fail !!"));
//     //     }


//     // }    




//         break;

//     default:

//         break;
//     }
// }

