#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
std::string getLocalLanIP()
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    // 获取所有网络接口信息，失败则返回空字符串
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs failed"); // 打印错误信息
        return "";
    }

    std::string result = "";

    // 遍历所有网络接口
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // 筛选IPv4地址（AF_INET）
        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            // 将二进制IP转换为字符串格式
            inet_ntop(AF_INET, addr, host, NI_MAXHOST);

            std::string ip = host;

            // 过滤回环地址（127.0.0.1），取第一个非回环的LAN IP
            if (ip != "127.0.0.1") {
                result = ip;
                break;
            }
        }
    }

    // 释放接口信息内存
    freeifaddrs(ifaddr);
    return result;
}


std::string getVpnIp()
{
    return "10.0.0.1";
}
