//PortUtils.h
#pragma once
#include <cstdint>

// 检测指定UDP端口是否可用（绑定成功则可用）
bool isUdpPortAvailable(uint16_t port);

// 查找指定范围内的可用UDP端口，无可用返回-1
// 默认范围：5000-65535（避开知名端口，适合音视频传输）
int findAvailableUdpPort(uint16_t startPort = 5000, uint16_t endPort = 65535);
