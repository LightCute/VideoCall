//LoginServer.cpp
// LoginServer: protocol ↔ domain 的边界层

#include "./core/LoginServer.h"



// 实现 LoginServer 构造函数
LoginServer::LoginServer() 
    : dispatcher_(loginService_, sessionMgr_)  // 手动初始化 dispatcher_，传入所需两个参数
{
    // 构造函数体可留空（如需其他初始化逻辑可补充）
}
LoginServer::~LoginServer() {
    listener_.stop();      // 停止 accept 线程
    // ThreadPool 析构会自动 join
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

void LoginServer::onMessage(int fd, const std::string& msg)
{
    ServerEvent event = ServerEventFactory::makeEvent(msg);

    auto actions = dispatcher_.dispatch(fd, event);

    for (auto& act : actions) {
        std::visit([this](auto&& a) {
            handle(a);
        }, act);
    }
}


// LoginServer.cpp

void LoginServer::handle(const SendLoginOk& a)
{
    proto::UserInfo u;
    u.username = a.username;
    u.privilege = a.privilege;

    auto payload = proto::makeLoginOk(u, "welcome");
    listener_.sendPacket(a.fd, payload);
}

void LoginServer::handle(const SendLoginFail& a)
{
    auto payload = proto::makeLoginFail(a.reason);
    listener_.sendPacket(a.fd, payload);
}

void LoginServer::handle(const BroadcastOnlineUsers&)
{
    auto snapshot = sessionMgr_.snapshot();

    proto::OnlineUsers users;
    for (auto& [fd, info] : snapshot) {
        const domain::User& u = info.user;

        users.users.push_back(proto::UserInfo{
            u.username,
            u.privilege
        });
    }


    auto payload = proto::makeOnlineUsers(users);

    std::cout << "Broadcast: " << payload << std::endl;
    for (auto& [fd, _] : snapshot) {
        listener_.sendPacket(fd, payload);
    }
}

void LoginServer::handle(const SendError& a)
{
    auto payload = (a.reason);
    std::cout << "Error: " << payload << std::endl;

    listener_.sendPacket(a.fd, payload);
}

