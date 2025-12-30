#include <iostream>
#include "gst_video.h"

int main() {
    CameraManager cam;

    // 1. 列出所有可用的摄像头
    std::cout << "=== 可用摄像头设备 ===" << std::endl;
    auto devs = cam.list_devices();
    for (int i = 0; i < devs.size(); i++) {
        std::cout << i << ": " << devs[i] << std::endl;
    }

    if (devs.empty()) {
        std::cout << "未检测到摄像头设备！" << std::endl;
        return 0;
    }

    // 2. 用户选择摄像头（示例直接选第 0 个）
    cam.set_device(devs[0]);
    std::cout << "选择摄像头：" << devs[0] << std::endl;

    // 3. 设置分辨率, FPS
    cam.set_resolution(640, 480);
    cam.set_fps(30);

    // 4. 选择编码方式：raw / mjpeg / x264
    cam.set_encoding("mjpeg");

    // 5. 设置软编码 / 硬编码
    cam.set_hw_encode(false); // false = 软件编码 

    // 6. 设置 UDP 目标 IP & 端口
    cam.set_udp_target("10.0.0.4", 5000);

    // 7. 启动
    std::cout << "启动 Pipeline..." << std::endl;
    if (!cam.start()) {
        std::cout << "Pipeline 启动失败！" << std::endl;
        return -1;
    }

    std::cout << "Pipeline 正在运行，按回车键停止..." << std::endl;
    std::cin.get(); // 等待用户按键退出

    // 8. 停止
    cam.stop();
    std::cout << "Pipeline 已停止。" << std::endl;

    return 0;
}
