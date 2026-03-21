#include "GstMediaPlayer.h"
#include <iostream>

GstMediaPlayer::GstMediaPlayer() 
    : m_pipeline(nullptr), m_appsrcVideo(nullptr), m_appsrcAudio(nullptr), m_isRunning(false) {
    gst_init(nullptr, nullptr);
}

GstMediaPlayer::~GstMediaPlayer() {
    stop();
}

bool GstMediaPlayer::start() {
    if (m_isRunning) return true;

    g_print("[GstMediaPlayer] Starting pipeline...\n");

    // ===============================================================
    // 核心 Pipeline 字符串
    // 注意：
    // 1. 视频：sync=false (低延迟)
    // 2. 音频：sync=false (先不管同步，只要出声！)
    // ===============================================================
    std::string pipelineDesc = 
        // ---------------- 视频支路 (保持原样) ----------------
        "appsrc name=video_src emit-signals=false is-live=true ! "
        "queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 ! " // 不限制队列
        "h264parse ! decodebin ! videoconvert ! autovideosink sync=false "
        
        // ---------------- 音频支路 (关键修改) ----------------
        "appsrc name=audio_src emit-signals=false is-live=true ! "
        "queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 ! "
        "opusparse ! " // 专门解析 Opus
        "opusdec ! "    // 专门解码 Opus
        "audioconvert ! "
        "audioresample ! "
        "autoaudiosink sync=false"; // sync=false 强制播放，不等待时钟

    GError* error = nullptr;
    m_pipeline = gst_parse_launch(pipelineDesc.c_str(), &error);
    if (error) {
        g_printerr("[GstMediaPlayer] Pipeline ERROR: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    // 获取 AppSrc
    m_appsrcVideo = gst_bin_get_by_name(GST_BIN(m_pipeline), "video_src");
    m_appsrcAudio = gst_bin_get_by_name(GST_BIN(m_pipeline), "audio_src");

    if (!m_appsrcVideo || !m_appsrcAudio) {
        g_printerr("[GstMediaPlayer] Failed to get AppSrc elements\n");
        return false;
    }

    // ===============================================================
    // 设置 Video Caps (保持不变)
    // ===============================================================
    GstCaps* videoCaps = gst_caps_new_simple(
        "video/x-h264",
        "stream-format", G_TYPE_STRING, "byte-stream",
        "alignment", G_TYPE_STRING, "au",
        nullptr
    );
    g_object_set(m_appsrcVideo, "caps", videoCaps, nullptr);
    gst_caps_unref(videoCaps);

    // ===============================================================
    // ✅✅✅ 设置 Audio Caps (关键！)
    // 这里我们不假设太多，只说这是 Opus
    // ===============================================================
    GstCaps* audioCaps = gst_caps_new_simple(
        "audio/x-opus",
        nullptr // 不添加任何额外限制，让 GStreamer 自适应
    );
    g_object_set(m_appsrcAudio, "caps", audioCaps, nullptr);
    gst_caps_unref(audioCaps);

    // 启动 Pipeline
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    
    // ===============================================================
    // 增加：监听 Pipeline 总线上的消息 (看报错)
    // ===============================================================
    GstBus* bus = gst_element_get_bus(m_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)GstMediaPlayer::onGstMessage, this);
    gst_object_unref(bus);

    m_isRunning = true;
    g_print("[GstMediaPlayer] Started successfully (A/V Sync disabled for testing)\n");
    return true;
}

// 新增：监听 GStreamer 错误消息
gboolean GstMediaPlayer::onGstMessage(GstBus* bus, GstMessage* msg, gpointer user_data) {
    (void)bus;
    (void)user_data;
    
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError* err;
            gchar* debug;
            gst_message_parse_error(msg, &err, &debug);
            g_printerr("[GstMediaPlayer] 🔴 ERROR: %s\n", err->message);
            g_printerr("[GstMediaPlayer] 🔴 Debug: %s\n", debug);
            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_WARNING: {
            GError* err;
            gchar* debug;
            gst_message_parse_warning(msg, &err, &debug);
            g_printerr("[GstMediaPlayer] 🟡 WARNING: %s\n", err->message);
            g_error_free(err);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
            // 忽略状态变化
            break;
        default:
            break;
    }
    return TRUE;
}

void GstMediaPlayer::stop() {
    if (!m_isRunning) return;
    m_isRunning = false;

    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_appsrcVideo = nullptr;
        m_appsrcAudio = nullptr;
    }
    g_print("[GstMediaPlayer] Stopped\n");
}

void GstMediaPlayer::pushVideoFrame(rtc::binary data, uint32_t timestamp) {
    if (!m_isRunning || !m_appsrcVideo) return;
    pushToAppSrc(m_appsrcVideo, data, timestamp, 90000); // 视频时钟 90kHz
}

void GstMediaPlayer::pushAudioFrame(rtc::binary data, uint32_t timestamp) {
    if (!m_isRunning || !m_appsrcAudio) return;
    pushToAppSrc(m_appsrcAudio, data, timestamp, 48000); // 音频时钟 48kHz (Opus 标准)
}

// 通用推送函数
void GstMediaPlayer::pushToAppSrc(GstElement* appsrc, const rtc::binary& data, uint32_t timestamp, uint32_t clockRate) {
    if (data.empty()) return;

    GstBuffer* buffer = gst_buffer_new_allocate(nullptr, data.size(), nullptr);
    if (!buffer) return;

    // 1. 拷贝数据
    GstMapInfo map;
    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
        memcpy(map.data, data.data(), data.size());
        gst_buffer_unmap(buffer, &map);
    } else {
        gst_buffer_unref(buffer);
        return;
    }

    // 2. 简单的时间戳设置 (可选，先不管，让 GStreamer 自己处理)
    GST_BUFFER_PTS(buffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DTS(buffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION(buffer) = GST_CLOCK_TIME_NONE;

    // 3. 推入数据
    GstFlowReturn ret;
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);
    
    if (ret != GST_FLOW_OK) {
        // 只有出错时才打印
        // g_printerr("[GstMediaPlayer] Push failed: %d\n", ret);
    }

    gst_buffer_unref(buffer);
}