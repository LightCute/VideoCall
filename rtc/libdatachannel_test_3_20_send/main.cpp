#include "DataChannelClient.h"
#include <iostream>
#include <string>
#include <thread>
#include <atomic>

// 跨平台非阻塞输入检查（简化版）
bool hasInput() {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    timeval tv{0, 100000}; // 100ms 超时
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
#endif
}

int main() {
    try {
        DataChannelClient dataChannelClient;

        // 连接信令服务器
        dataChannelClient.connectToServer("ws://120.79.210.6:8000");
        Log::info("[Main] Local ID: {}", dataChannelClient.getLocalId());
        Log::info("[Main] You can enter a peer ID to call, or wait for incoming connections.");
            std::string currentPeerId;  // 跟踪当前连接的 peer
        bool isConnecting = false;  // 跟踪是否正在连接中
        // 主循环：同时支持主动呼叫和被动接收
               while (true) {
            // ✅ 修复 1: 使用更准确的连接检测
            if (dataChannelClient.hasActiveConnection()) {
                // 获取已连接的 peer ID
                auto connectedPeers = dataChannelClient.getConnectedPeers();
                if (!connectedPeers.empty()) {
                    currentPeerId = connectedPeers[0];
                    Log::info("[Main] Connected to peer: {}", currentPeerId);
                    
                    // 消息发送循环
                    while (true) {
                        Log::info("[Main] Waiting for user input (empty to disconnect)...");
                        std::string msg;
                        std::getline(std::cin, msg);
                        
                        if (msg.empty()) {
                            Log::info("[Main] Disconnecting...");
                            dataChannelClient.close();
                            currentPeerId.clear();
                            isConnecting = false;
                            break;
                        }
                        
                        // ✅ 修复 2: 发送前检查 DataChannel 状态
                        if (dataChannelClient.isDataChannelOpen(currentPeerId)) {
                            dataChannelClient.sendMessage(currentPeerId, msg);
                        } else {
                            Log::warn("[Main] DataChannel not open, cannot send!");
                        }
                    }
                    continue;
                }
            }

            // ✅ 修复 3: 防止重复呼叫
            if (isConnecting) {
                Log::info("[Main] Connecting to peer, please wait...");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // 检查用户输入
            if (hasInput()) {
                std::string peerId;
                std::cin >> peerId;
                std::cin.ignore();
                
                if (peerId.empty()) {
                    Log::info("[Main] Exiting...");
                    break;
                }
                
                // ✅ 修复 4: 检查是否已连接到此 peer
                if (dataChannelClient.isDataChannelOpen(peerId)) {
                    Log::warn("[Main] Already connected to {}", peerId);
                    currentPeerId = peerId;
                    continue;
                }
                
                // 发起呼叫
                dataChannelClient.callPeer(peerId);
                isConnecting = true;
                Log::info("[Main] Called {}, waiting for connection...", peerId);
                
                // ✅ 修复 5: 等待连接完成或超时
                if (dataChannelClient.waitForConnection(std::chrono::seconds(10))) {
                    Log::info("[Main] Connection established!");
                    isConnecting = false;
                } else {
                    Log::error("[Main] Connection timeout!");
                    isConnecting = false;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        return 0;
    } catch (const std::exception& e) {
        Log::error("[Main] Exception: {}", e.what());
        return -1;
    }

    spdlog::shutdown();
}