//PacketCodec.h
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>

class PacketCodec {
public:
    // 打包：payload → 带长度头的二进制数据
    static std::vector<char> encode(const std::string& payload) {
        uint32_t len = payload.size();
        uint32_t netLen = htonl(len);

        std::vector<char> buf(sizeof(netLen) + len);
        std::memcpy(buf.data(), &netLen, sizeof(netLen));
        std::memcpy(buf.data() + sizeof(netLen), payload.data(), len);
        return buf;
    }

    // 尝试解包：从 buffer 中提取一个完整 payload
    // 成功返回 true，并填充 outPayload
    static bool tryDecode(std::vector<char>& buffer, std::string& outPayload) {
        if (buffer.size() < 4)
            return false;

        uint32_t netLen;
        std::memcpy(&netLen, buffer.data(), 4);
        uint32_t len = ntohl(netLen);

        if (buffer.size() < 4 + len)
            return false;

        outPayload.assign(buffer.data() + 4, len);
        buffer.erase(buffer.begin(), buffer.begin() + 4 + len);
        return true;
    }
};
