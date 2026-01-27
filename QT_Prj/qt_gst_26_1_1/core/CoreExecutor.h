//CoreExecutor.h
#pragma once
#include "CoreInput.h"
#include "CommandSocket.h"
#include "protocol_text.h"
#include "ClientEventFactory.h"
#include <functional>
#include <memory>

// 定义回调类型：Executor 执行 IO 后，通过该回调向 ClientCore 推送 CoreInput
using InputCallback = std::function<void(core::CoreInput)>;
enum class NetMode { LAN, VPN };

class CoreExecutor {
public:
    // 构造函数：接收回调函数（用于向 Core 推送输入事件）
    explicit CoreExecutor(InputCallback cb);
    ~CoreExecutor();

    // 对外暴露的 IO 操作接口（语义化命名）
    void connectToServer(const std::string& host, int port);
    void sendLoginRequest(const std::string& user, const std::string& pass);
    void sendPing();
    void stop(); // 资源释放接口
    void setLanMode() ;
    void setVpnMode() ;
    std::string selectPeerIp(const std::string& lanIp,
                             const std::string& vpnIp) const;
    void sendLocalIP();
    void sendTextMsg(const std::string& target_user, const std::string& content);
    void sendCallRequest(const std::string& target_user);
    void sendAcceptCall();
    void sendRejectCall();
    void sendMediaOffer(const std::string& peer);
    void sendMediaAnswer(const std::string& peer);
    int getMediaPort() const { return mediaPort_; }
private:
    // 封装的 socket 实例（Executor 唯一持有）
    CommandSocket socket_;
    // 向 Core 推送事件的回调
    InputCallback postInput_;
    // 线程安全标记
    std::atomic<bool> isRunning_{true};

    // 初始化 socket 回调（内部私有）
    void initSocketCallbacks();

    std::thread heartbeatThread_;
    void heartbeatLoop();   // 新增


    std::atomic<NetMode> mode_{NetMode::LAN};
    std::atomic<int> mediaPort_{0}; // 初始值0表示未选择

};
