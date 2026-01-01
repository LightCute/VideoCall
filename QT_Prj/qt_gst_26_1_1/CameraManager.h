#pragma once

#include <string>
#include <functional>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include "Frame.h"

class CameraManager {
public:
    using FrameCallback = std::function<void(const Frame&)>;

    CameraManager();
    ~CameraManager();

    bool start(const std::string& device);
    void stop();

    void setFrameCallback(FrameCallback cb);

private:
    GstElement* pipeline_ = nullptr;
    GstElement* appsink_  = nullptr;

    FrameCallback callback_;

    static GstFlowReturn onNewSample(GstAppSink* sink, gpointer user_data);
};
