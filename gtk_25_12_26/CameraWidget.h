#pragma once

#include <gst/gst.h>

#include <string>

class CameraWidget {
public:
    CameraWidget();
    ~CameraWidget();

    void set_device(const std::string& dev);
    bool is_running() const;
    void restart();
    // 初始化 pipeline（不 PLAYING）
    bool init();

    // 获取 gtksink 的 widget（用于嵌入 UI）
    GtkWidget* get_widget();

    // 启停 pipeline
    bool start();
    void stop();

private:
    std::string device_;

    GstElement* pipeline_;
    GstElement* src_;
    GstElement* convert_;
    GstElement* sink_;

    GtkWidget* video_widget_;
};
