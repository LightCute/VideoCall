#include "CameraWidget.h"
#include <iostream>

CameraWidget::CameraWidget()
    : pipeline_(nullptr),
      src_(nullptr),
      convert_(nullptr),
      sink_(nullptr),
      video_widget_(nullptr)
{
    gst_init(nullptr, nullptr);
}

CameraWidget::~CameraWidget() {
    stop();
}

void CameraWidget::set_device(const std::string& dev) {
    device_ = dev;
}

bool CameraWidget::init() {
    if (device_.empty()) {
        std::cerr << "No device set\n";
        return false;
    }

    pipeline_ = gst_pipeline_new("camera-pipeline");
    src_      = gst_element_factory_make("v4l2src", "src");
    convert_  = gst_element_factory_make("videoconvert", "convert");
    sink_ = gst_element_factory_make("glimagesink", "sink");

    if (!pipeline_ || !src_ || !convert_ || !sink_) {
        std::cerr << "Failed to create GStreamer elements\n";
        return false;
    }

    g_object_set(src_, "device", device_.c_str(), NULL);

    gst_bin_add_many(GST_BIN(pipeline_), src_, convert_, sink_, NULL);

    if (!gst_element_link_many(src_, convert_, sink_, NULL)) {
        std::cerr << "Failed to link pipeline\n";
        return false;
    }

    // ⚠️ 注意：这里只 get，不 set
    g_object_get(sink_, "widget", &video_widget_, NULL);

    return true;
}

GtkWidget* CameraWidget::get_widget() {
    return video_widget_;
}

bool CameraWidget::start() {
    if (!pipeline_) {
        if (!init()) return false;  // 如果 pipeline 不存在，先初始化
    }
    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    return true;
}

void CameraWidget::stop() {
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
    video_widget_ = nullptr;
}


bool CameraWidget::is_running() const {
    if (!pipeline_) return false;
    GstState state;
    gst_element_get_state(pipeline_, &state, NULL, 0);
    return state == GST_STATE_PLAYING;
}

void CameraWidget::restart() {
    stop();
    init();
    start();
}





