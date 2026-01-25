//VideoReceiver.cpp
#include "VideoReceiver.h"
#include <iostream>
#include <cstring>

VideoReceiver::VideoReceiver() {
    // 初始化GStreamer（和CameraManager保持一致）
    gst_init(nullptr, nullptr);
}

VideoReceiver::~VideoReceiver() {
    stop(); // 析构时确保停止pipeline
}

void VideoReceiver::setFrameCallback(FrameCallback cb) {
    callback_ = std::move(cb);
}

bool VideoReceiver::start(int listenPort) {
    // 校验端口合法性
    if ((listenPort < 1) || (listenPort > 65535)) {
        std::cerr << "[VideoReceiver] Invalid listen port: " << listenPort << std::endl;
        return false;
    }

    if (pipeline_) return true; // 避免重复启动

    // 核心：接收端pipeline字符串（appsink版，适配Qt RGB显示）
    std::string pipelineStr =
        "udpsrc port=" + std::to_string(listenPort) +
        " caps=\"application/x-rtp,media=video,encoding-name=H264,payload=96\" "
        " ! rtph264depay "          // RTP解包（去掉RTP头）
        " ! h264parse "             // H264解析（保证解码器输入稳定）
        " ! avdec_h264 "            // H264软件解码（兼容所有平台）
        " ! videoconvert "          // 颜色空间转换（解码后可能是YUV，转RGB）
        " ! video/x-raw,format=RGB "// 和Qt显示格式完全一致（RGB888）
        " ! appsink name=recv_sink "; // 接收端appsink（命名为recv_sink，避免和发送端冲突）

    // 创建pipeline
    pipeline_ = gst_parse_launch(pipelineStr.c_str(), nullptr);
    if (!pipeline_) {
        std::cerr << "[VideoReceiver] Failed to create pipeline" << std::endl;
        return false;
    }

    // 配置appsink（和CameraManager逻辑完全复用）
    appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "recv_sink");
    gst_app_sink_set_emit_signals(GST_APP_SINK(appsink_), true);
    gst_app_sink_set_drop(GST_APP_SINK(appsink_), true);
    gst_app_sink_set_max_buffers(GST_APP_SINK(appsink_), 1); // 只保留最新帧，避免卡顿

    // 绑定new-sample信号（帧回调）
    g_signal_connect(appsink_, "new-sample",
                     G_CALLBACK(onNewSample), this);

    // 启动pipeline
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    std::cout << "[VideoReceiver] Started successfully, listen on UDP port: " << listenPort << std::endl;
    return true;
}

void VideoReceiver::stop() {
    if (!pipeline_) return;

    // 停止pipeline并释放资源（GStreamer标准操作）
    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
    appsink_ = nullptr;
}

GstFlowReturn VideoReceiver::onNewSample(GstAppSink* sink, gpointer user_data) {
    // 完全复用CameraManager的帧处理逻辑（保证和发送端格式一致）
    auto* self = static_cast<VideoReceiver*>(user_data);
    if (!self->callback_) return GST_FLOW_OK;

    GstSample* sample = gst_app_sink_pull_sample(sink);
    GstBuffer* buffer = gst_sample_get_buffer(sample);
    GstCaps* caps = gst_sample_get_caps(sample);

    GstStructure* s = gst_caps_get_structure(caps, 0);
    int width = 0, height = 0;
    gst_structure_get_int(s, "width", &width);
    gst_structure_get_int(s, "height", &height);

    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_READ);

    Frame frame;
    frame.width = width;
    frame.height = height;
    frame.stride = width * 3; // RGB888：每行字节数=宽度×3
    frame.format = PixelFormat::RGB888;
    frame.timestamp = GST_BUFFER_PTS(buffer);

    frame.data.resize(map.size);
    std::memcpy(frame.data.data(), map.data, map.size);

    // 调用回调，把帧推给Qt UI
    self->callback_(frame);

    // 释放资源（GStreamer内存管理必须做）
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}
