#pragma once
#include <vector>
#include <cstdint>
#include <QMetaType>   // ⚠️ 必须加这一行
enum class PixelFormat {
    RGB888
};

struct Frame {
    std::vector<uint8_t> data; // 连续内存
    int width = 0;
    int height = 0;
    int stride = 0;
    PixelFormat format;
    uint64_t timestamp = 0;
};

Q_DECLARE_METATYPE(Frame)
