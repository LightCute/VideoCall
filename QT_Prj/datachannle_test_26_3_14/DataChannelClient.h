#pragma once

#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <future>
#include "utilities/log.h"

class DataChannelClient {
public:
    DataChannelClient();
    ~DataChannelClient();

    // 禁止拷贝和移动（确保资源安全）
    DataChannelClient(const DataChannelClient&) = delete;
    DataChannelClient& operator=(const DataChannelClient&) = delete;
    DataChannelClient(DataChannelClient&&) = delete;
    DataChannelClient& operator=(DataChannelClient&&) = delete;

    // 连接信令服务器
    void connectToServer(const std::string& serverUrl);

    // 向指定 Peer 发送 Offer 并建立连接
    void callPeer(const std::string& peerId);

    // 向指定 Peer 发送文本消息
    void sendMessage(const std::string& peerId, const std::string& message);

    // 关闭所有连接并清理资源
    void close();

    // 获取本地生成的 ID
    std::string getLocalId() const;

private:
    // 创建并配置 PeerConnection
    std::shared_ptr<rtc::PeerConnection> createPeerConnection(
        std::weak_ptr<rtc::WebSocket> wws, 
        const std::string& id
    );

    // 生成随机 ID
    static std::string randomId(size_t length);

    // 枚举转字符串辅助函数
    static std::string peerConnectionStateToString(rtc::PeerConnection::State state);
    static std::string gatheringStateToString(rtc::PeerConnection::GatheringState state);

    // RTC 日志回调
    static void myCppLogCallback(rtc::LogLevel level, std::string message);

    // 核心成员变量
    std::string m_localId;
    rtc::Configuration m_config;
    std::shared_ptr<rtc::WebSocket> m_ws;
    std::unordered_map<std::string, std::shared_ptr<rtc::PeerConnection>> m_peerConnectionMap;
    std::unordered_map<std::string, std::shared_ptr<rtc::DataChannel>> m_dataChannelMap;
    std::promise<void> m_wsPromise; // 用于等待 WebSocket 连接
};