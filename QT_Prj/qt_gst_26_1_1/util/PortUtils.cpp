//PortUtils.cpp
#include "PortUtils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

bool isUdpPortAvailable(uint16_t port) {
    // 1. 创建UDP套接字（SOCK_DGRAM）
    // 原因：UDP端口检测必须用UDP套接字，TCP检测结果不适用
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket for port check: " << port << std::endl;
        return false;
    }

    // 2. 设置端口复用
    // 原因：避免TIME_WAIT状态导致的端口暂时不可用，提高检测准确性
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. 尝试绑定端口
    // 原因：UDP端口是否可用的核心判断依据（能绑定则未被占用）
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY; // 绑定所有网卡

    bool available = (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == 0);

    // 4. 关闭套接字
    // 原因：仅检测可用性，无需保留套接字，释放资源
    close(sockfd);

    return available;
}

int findAvailableUdpPort(uint16_t startPort, uint16_t endPort) {
    // 遍历端口范围，返回第一个可用端口
    // 原因：优先使用低端口（5000+），符合音视频传输的常用端口习惯
    for (uint16_t port = startPort; port <= endPort; ++port) {
        if (isUdpPortAvailable(port)) {
            std::cout << "[PortUtils] Found available UDP port: " << port << std::endl;
            return port;
        }
    }
    std::cerr << "[PortUtils] No available UDP port in range [" << startPort << ", " << endPort << "]" << std::endl;
    return -1;
}
