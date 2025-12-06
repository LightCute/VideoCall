#include "gst_video.h"
#include <iostream>
#include <filesystem>

using namespace std;

CameraManager::CameraManager()
    : pipeline(nullptr),
      width(640),
      height(480),
      fps(30),
      encoding("x264"),
      use_hw(false),
      target_ip("127.0.0.1"),
      target_port(5000),
      running(false)
{
    gst_init(nullptr, nullptr);
}

CameraManager::~CameraManager() {
    stop();
}

vector<string> CameraManager::list_devices() {
    vector<string> result;
    for (const auto &entry : filesystem::directory_iterator("/dev")) {
        if (entry.path().string().find("video") != string::npos)
            result.push_back(entry.path().string());
    }
    return result;
}

void CameraManager::set_device(const string& dev) { device = dev; }
void CameraManager::set_resolution(int w, int h) { width = w; height = h; }
void CameraManager::set_fps(int f) { fps = f; }
void CameraManager::set_encoding(const string& codec) { encoding = codec; }
void CameraManager::set_hw_encode(bool enable) { use_hw = enable; }
void CameraManager::set_udp_target(const string& ip, int port) {
    target_ip = ip;
    target_port = port;
}

bool CameraManager::start() {
    if (running) stop();
    if (!build_pipeline()) return false;

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    running = true;
    return true;
}

void CameraManager::stop() {
    if (!running) return;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = nullptr;
    running = false;
}

bool CameraManager::is_running() { return running; }

void CameraManager::clear_pipeline() {
    if (pipeline) {
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
}

// ----------------------------
// 最核心：构建 GStreamer pipeline
// ----------------------------

bool CameraManager::build_pipeline() {
    clear_pipeline();

    // 根据用户设置生成 caps
    char caps_str[200];
    sprintf(caps_str,
            "video/x-raw,width=%d,height=%d,framerate=%d/1",
            width, height, fps);

    // 选择编码器
    string encoder;
    if (encoding == "raw") {
        encoder = "videoconvert ! queue";
    } 
    else if (encoding == "mjpeg") {
        encoder = "jpegenc";
    } 
    else if (encoding == "x264") {
        encoder = use_hw ? "v4l2h264enc extra-controls=\"encode,bitrate=2000000\""
                         : "x264enc tune=zerolatency bitrate=2000 speed-preset=ultrafast";
    }

    // 构建完整 pipeline 字符串
    string pipe = 
        "v4l2src device=" + device + " ! " +
        caps_str + " ! videoconvert ! " +
        encoder + " ! " +
        "rtpjpegpay ! "
        "udpsink host=" + target_ip +
        " port=" + to_string(target_port) + " sync=false";

    GError *err = nullptr;
    pipeline = gst_parse_launch(pipe.c_str(), &err);

    if (!pipeline || err) {
        cerr << "Pipeline build error: " << (err ? err->message : "") << endl;
        if (err) g_error_free(err);
        return false;
    }

    return true;
}
