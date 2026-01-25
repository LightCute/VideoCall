//VideoReceiver.h
#pragma once

#include <string>
#include <functional>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "Frame.h"

// 和CameraManager完全对称的接收类
class VideoReceiver {
public:
    using FrameCallback = std::function<void(const Frame&)>;

    VideoReceiver();
    ~VideoReceiver();

    // 设置帧回调（推给Qt UI）
    void setFrameCallback(FrameCallback cb);
    // 启动接收（监听指定UDP端口）
    bool start(int listenPort);
    // 停止接收
    void stop();

private:
    // appsink回调函数（复用CameraManager的逻辑）
    static GstFlowReturn onNewSample(GstAppSink* sink, gpointer user_data);

private:
    GstElement* pipeline_ = nullptr;  // 接收端独立pipeline
    GstElement* appsink_  = nullptr;  // 接收端appsink
    FrameCallback callback_;          // 帧回调
};
