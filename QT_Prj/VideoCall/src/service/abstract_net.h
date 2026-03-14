#pragma once
#include <functional>
#include <string>

class AbstractNet {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using OpenCallback = std::function<void()>;
    using CloseCallback = std::function<void()>;
    using ErrorCallback = std::function<void(const std::string&)>;

    virtual ~AbstractNet() = default;

    // 生命周期
    virtual void connect(const std::string& url) = 0;
    virtual void send(const std::string& msg) = 0;
    virtual void close() = 0;

    // 回调注册
    virtual void onMessage(MessageCallback cb) = 0;
    virtual void onOpen(OpenCallback cb) = 0;
    virtual void onClose(CloseCallback cb) = 0;
    virtual void onError(ErrorCallback cb) = 0;

    virtual void connect2Peer(const std::string& peerId) =0;
    virtual void send2Peer(const std::string& peerId, const std::string& msg) =0;
};