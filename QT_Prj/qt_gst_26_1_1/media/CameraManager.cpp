//CameraManager.cpp
#include "CameraManager.h"
#include <iostream>
#include <cstring>

CameraManager::CameraManager() {
    gst_init(nullptr, nullptr);
}

CameraManager::~CameraManager() {
    stop();
}

void CameraManager::setFrameCallback(FrameCallback cb) {
    callback_ = std::move(cb);
}

bool CameraManager::start(const std::string& device) {
    if (pipeline_) return true;
    std::cout << "[CameraManager] Attempting to start camera: " << device << std::endl;
    std::string pipelineStr =
        "v4l2src device=" + device +
        " ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 "
        " ! videoconvert "
        " ! openh264enc bitrate=2000000 "
        " ! h264parse "
        " ! tee name=t "
        " t. ! queue "
        "     ! avdec_h264 "
        "     ! videoconvert "
        "     ! video/x-raw,format=RGB "
        "     ! appsink name=sink "
        " t. ! queue "
        "     ! rtph264pay pt=96 config-interval=1 "
        "     ! udpsink host=192.168.6.15 port=5000 sync=false async=false";

    pipeline_ = gst_parse_launch(pipelineStr.c_str(), nullptr);
    if (!pipeline_) {
        std::cerr << "Failed to create pipeline\n";
        return false;
    }

    /* appsink 用于 Qt 显示 */
    appsink_ = gst_bin_get_by_name(GST_BIN(pipeline_), "sink");

    gst_app_sink_set_emit_signals(GST_APP_SINK(appsink_), true);
    gst_app_sink_set_drop(GST_APP_SINK(appsink_), true);
    gst_app_sink_set_max_buffers(GST_APP_SINK(appsink_), 1);
    
    g_signal_connect(appsink_, "new-sample",
                     G_CALLBACK(onNewSample), this);

    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    std::cout << "[CameraManager] Camera started successfully: " << device << std::endl;
    return true;
}

void CameraManager::stop() {
    if (!pipeline_) return;

    gst_element_set_state(pipeline_, GST_STATE_NULL);
    gst_object_unref(pipeline_);
    pipeline_ = nullptr;
    appsink_ = nullptr;
}

GstFlowReturn CameraManager::onNewSample(GstAppSink* sink, gpointer user_data) {
    auto* self = static_cast<CameraManager*>(user_data);
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
    frame.stride = width * 3;
    frame.format = PixelFormat::RGB888;
    frame.timestamp = GST_BUFFER_PTS(buffer);

    frame.data.resize(map.size);
    std::memcpy(frame.data.data(), map.data, map.size);

    self->callback_(frame);

    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}
