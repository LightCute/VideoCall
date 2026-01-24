//LoginServer.cpp
// LoginServer: protocol ↔ domain 的边界层

#include "./core/LoginServer.h"

// 实现 LoginServer 构造函数
LoginServer::LoginServer() 
    : dispatcher_(loginService_, sessionMgr_, callService_)  // 手动初始化 dispatcher_，传入所需两个参数
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
    bool ok = listener_.startListen(port);

    if (ok) {
        startHeartbeatMonitor(); // ⚡ 启动心跳监测
    }

    return ok;
}

void LoginServer::startHeartbeatMonitor() {
    std::thread([this] {
        while (true) {
            auto snapshot = sessionMgr_.snapshot();
            auto now = std::chrono::steady_clock::now();

            for (auto& [fd, info] : snapshot) {
                auto dur = std::chrono::duration_cast<std::chrono::seconds>(
                    now - info.lastHeartbeat
                ).count();

                if (dur > 15) { // 超过 15 秒没心跳就下线
                    std::cout << "[Server] heartbeat timeout fd=" << fd << std::endl;
                    sessionMgr_.logout(fd);
                    close(fd);

                    // 广播新的在线用户列表
                    auto onlineSnapshot = sessionMgr_.snapshot();
                    proto::OnlineUsers users;
                    for (auto& [fd2, info2] : onlineSnapshot) {
                        users.users.push_back({info2.user.username, info2.user.privilege});
                    }
                    auto payload = proto::makeOnlineUsers(users);
                    for (auto& [fd2, _] : onlineSnapshot) {
                        listener_.sendPacket(fd2, payload);
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }).detach();
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
        if (n <= 0) {
            break;}

        recvBuffer.insert(recvBuffer.end(), buf, buf + n);

        std::string payload;
        while (PacketCodec::tryDecode(recvBuffer, payload)) {
            onMessage(clientfd, payload);
        }
    }

    

    // 客户端断开处理
    sessionMgr_.logout(clientfd);

    auto snapshot = sessionMgr_.snapshot();
    proto::OnlineUsers users;
    for (auto& [fd, info] : snapshot) {
        users.users.push_back({info.user.username, info.user.privilege});
    }
    auto payload = proto::makeOnlineUsers(users);
    for (auto& [fd, _] : snapshot) {
        listener_.sendPacket(fd, payload);
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

// 处理发送新来电通知的Action
void LoginServer::handle(const SendCallIncoming& a) {
    // 构建新来电的payload
    auto payload = proto::makeCallIncoming(a.from_user);
    // 发送给被呼叫方
    listener_.sendPacket(a.target_fd, payload);
    std::cout << "[Server] Send call incoming to fd=" << a.target_fd 
              << " from=" << a.from_user << std::endl;
}

// 处理发送通话接通通知的Action
void LoginServer::handle(const SendCallAccepted& a) {
    // 构建通话接通的payload
    auto payload = proto::makeCallAccepted(a.peer);
    // 发送给目标用户
    listener_.sendPacket(a.fd, payload);
    std::cout << "[Server] Send call accepted to fd=" << a.fd 
              << " peer=" << a.peer << std::endl;
}

// 处理发送通话被拒通知的Action
void LoginServer::handle(const SendCallRejected& a) {
    // 构建通话被拒的payload
    auto payload = proto::makeCallRejected(a.peer);
    // 发送给呼叫方
    listener_.sendPacket(a.fd, payload);
    std::cout << "[Server] Send call rejected to fd=" << a.fd 
              << " peer=" << a.peer << std::endl;
}




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

void LoginServer::handle(const BroadcastLogout&)
{
    auto snapshot = sessionMgr_.snapshot();

    proto::OnlineUsers users;
    for (auto& [fd, info] : snapshot) {
        const domain::User& u = info.user;
        users.users.push_back({u.username, u.privilege});
    }

    auto payload = proto::makeOnlineUsers(users);

    for (auto& [fd, _] : snapshot) {
        listener_.sendPacket(fd, payload);
    }
}

void LoginServer::handle(const SendHeartbeatAck& a)
{
    std::cout << "[Server] Send PONG to fd= " << a.fd << std::endl;
    std::string payload = proto::makeHeartbeatAck(); // ⭐ PONG

    listener_.sendPacket(a.fd, payload);
}

void LoginServer::handle(const UpdatePeerInfo& a)
{
    ClientNetInfo net{
        .lanIp = a.lanIp,
        .vpnIp = a.vpnIp,
        .udpPort = a.udpPort
    };

    sessionMgr_.updateNetInfo(a.fd, net);

    std::cout << "[Server] Registered peer info for fd=" << a.fd << std::endl;
}


void LoginServer::handle(const SendUserNotFound& a)
{
    std::string reason = "User not found: " + a.target_user;
    auto payload = proto::makeLoginFail(reason); // 复用 LoginFail 格式返回错误
    listener_.sendPacket(a.fd, payload);
    std::cout << "[Server] Send user not found to fd=" << a.fd << ": " << a.target_user << std::endl;
}

// 调整：定向转发文本消息给目标用户
void LoginServer::handle(const ForwardText& a)
{
    // 目标用户不存在则跳过
    if (a.target_fd == -1) return;

    // 构建转发文本的 payload
    auto payload = proto::makeForwardTextMsg(a.from_user, a.content);

    // 仅发送给目标 FD
    listener_.sendPacket(a.target_fd, payload);
    std::cout << "[Server] Forward text to fd=" << a.target_fd 
              << " from=" << a.from_user 
              << " content=" << a.content << std::endl;
}
