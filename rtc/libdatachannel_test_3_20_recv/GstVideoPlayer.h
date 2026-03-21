#include <gst/gst.h>
#include <thread>
#include <atomic>
#include "rtc/rtc.hpp" // 确保包含 rtc::binary 的定义
#include "ThreadSafeQueue.h"
class GstVideoPlayer {
public:
    GstVideoPlayer() : m_isRunning(false) {
        gst_init(nullptr, nullptr);
        m_pipeline = nullptr;
        m_appsrc = nullptr;
    }

    ~GstVideoPlayer() {
        stop();
    }

    // 启动播放器和流水线
    bool start() {
        if (m_isRunning) return true;

        // 1. 创建 Pipeline
        // 注意：
        // appsrc: 我们的入口
        // h264parse: 解析 H.264 流
        // avdec_h264 / decodebin: 解码 (decodebin 自动选择)
        // videoconvert: 格式转换
        // autovideosink: 自动显示
        // sync=false: 实时流，不做时钟同步，降低延迟
        std::string pipelineDesc = 
            "appsrc name=src emit-signals=false is-live=true format=GST_FORMAT_TIME ! "
            "queue ! " // 加一个 queue 缓冲，防止抖动
            "h264parse ! "
            "decodebin ! "
            "videoconvert ! "
            "autovideosink sync=false";

        GError* error = nullptr;
        m_pipeline = gst_parse_launch(pipelineDesc.c_str(), &error);
        if (error) {
            g_printerr("[GstPlayer] Failed to create pipeline: %s\n", error->message);
            g_error_free(error);
            return false;
        }

        // 2. 获取 AppSrc
        m_appsrc = gst_bin_get_by_name(GST_BIN(m_pipeline), "src");
        if (!m_appsrc) {
            g_printerr("[GstPlayer] Failed to get AppSrc\n");
            return false;
        }

        // 3. 配置 AppSrc Caps
        // 告诉 GStreamer 我们要喂的是 H.264 Byte Stream (Annex-B)
        GstCaps* caps = gst_caps_new_simple(
            "video/x-h264",
            "stream-format", G_TYPE_STRING, "byte-stream",
            "alignment", G_TYPE_STRING, "au",
            nullptr
        );
        g_object_set(m_appsrc, "caps", caps, nullptr);
        gst_caps_unref(caps);

        // 4. 启动 Pipeline
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

        // 5. 启动工作线程
        m_isRunning = true;
        m_workerThread = std::thread(&GstVideoPlayer::workerLoop, this);

        g_print("[GstPlayer] Started successfully\n");
        return true;
    }

    // 供外部调用：推入帧数据 (非阻塞)
    void pushFrame(rtc::binary data) {
        if (!m_isRunning) return;
        m_queue.push(std::move(data));
    }

    // 停止播放器
    void stop() {
        if (!m_isRunning) return;
        
        g_print("[GstPlayer] Stopping...\n");
        
        // 1. 停止队列，唤醒工作线程
        m_queue.stop();

        // 2. 等待工作线程退出
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }

        // 3. 停止 GStreamer
        if (m_pipeline) {
            gst_element_set_state(m_pipeline, GST_STATE_NULL);
            gst_object_unref(m_pipeline);
            m_pipeline = nullptr;
            m_appsrc = nullptr;
        }

        m_isRunning = false;
        g_print("[GstPlayer] Stopped\n");
    }

private:
    // 工作线程主循环
    void workerLoop() {
        g_print("[GstPlayer] Worker thread started\n");
        
        rtc::binary data;
        
        // 循环从队列取数据
        while (m_queue.pop(data)) {
            if (data.empty()) continue;
            
            pushToGst(data);
        }
        
        g_print("[GstPlayer] Worker thread finished\n");
    }

    // 将数据真正推给 GStreamer
    void pushToGst(const rtc::binary& data) {
        if (!m_appsrc || !m_isRunning) return;

        // 1. 分配 GstBuffer
        GstBuffer* buffer = gst_buffer_new_allocate(nullptr, data.size(), nullptr);
        if (!buffer) return;

        // 2. 拷贝数据
        GstMapInfo map;
        if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
            memcpy(map.data, data.data(), data.size());
            gst_buffer_unmap(buffer, &map);
        } else {
            gst_buffer_unref(buffer);
            return;
        }

        // 3. 推送到 AppSrc
        GstFlowReturn ret;
        g_signal_emit_by_name(m_appsrc, "push-buffer", buffer, &ret);
        
        if (ret != GST_FLOW_OK) {
            // 非致命错误，可能是网络抖动或正在关闭
            // g_printerr("[GstPlayer] Push failed: %d\n", ret);
        }

        gst_buffer_unref(buffer);
    }

    // 成员变量
    GstElement* m_pipeline;
    GstElement* m_appsrc;
    
    ThreadSafeQueue<rtc::binary> m_queue;
    std::thread m_workerThread;
    std::atomic<bool> m_isRunning;
};