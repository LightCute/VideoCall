#ifndef GST_VIDEO_H
#define GST_VIDEO_H

#include <gst/gst.h>
#include <string>
#include <vector>

class CameraManager {
public:
    CameraManager();
    ~CameraManager();

    // ----------- 摄像头相关 -----------
    std::vector<std::string> list_devices();              // 列出摄像头设备
    void set_device(const std::string& dev);              // 设置摄像头设备

    // ----------- 视频参数 -----------
    void set_resolution(int w, int h);
    void set_fps(int fps);
    void set_encoding(const std::string& codec);          // raw, mjpeg, x264
    void set_hw_encode(bool enable);                      // 切换软/硬编码

    // ----------- 网络参数 -----------
    void set_udp_target(const std::string& ip, int port); // 设置IP与端口号

    // ----------- 控制 -----------
    bool start();
    void stop();
    bool is_running();

private:
    bool build_pipeline();  
    void clear_pipeline();

private:
    GstElement *pipeline;

    // 摄像头配置
    std::string device;
    int width;
    int height;
    int fps;
    std::string encoding;
    bool use_hw;

    // 网络配置
    std::string target_ip;
    int target_port;

    bool running;
};

#endif
