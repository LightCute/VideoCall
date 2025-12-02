#include <opencv2/opencv.hpp>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main() {
    // ------------------------
    // 打开摄像头
    // ------------------------
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);

    if(!cap.isOpened()) {
        std::cerr << "无法打开摄像头" << std::endl;
        return -1;
    }

    // 设置 MJPG 编码
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    // 设置最小分辨率
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    int width  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "摄像头分辨率: " << width << "x" << height << std::endl;

    // ------------------------
    // 创建 UDP 套接字
    // ------------------------
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "无法创建 socket\n";
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);

    // 使用 Docker 容器名
    if(inet_pton(AF_INET, "10.0.0.4", &addr.sin_addr) <= 0) { // 可替换成容器名的 IP
        std::cerr << "IP 地址无效\n";
        return -1;
    }

    std::vector<uchar> jpgBuf;
    std::vector<int> encode_params = {cv::IMWRITE_JPEG_QUALITY, 50}; // 压缩质量 50%

    cv::Mat frame;

    while(true) {
        if(!cap.read(frame) || frame.empty()) {
            std::cerr << "捕获帧失败" << std::endl;
            continue;
        }

        // 显示摄像头画面
        cv::imshow("Camera", frame);

        // JPEG 压缩
        jpgBuf.clear();
        cv::imencode(".jpg", frame, jpgBuf, encode_params);

        // 检查 UDP 包大小
        if(jpgBuf.size() > 60000) {
            std::cerr << "警告: 单帧数据过大: " << jpgBuf.size() << " 字节" << std::endl;
            continue; // 丢弃或切分发送
        }

        // 发送 UDP 数据
        ssize_t sent = sendto(sock,
                              jpgBuf.data(),
                              jpgBuf.size(),
                              0,
                              (sockaddr*)&addr,
                              sizeof(addr));

        if(sent < 0) {
            perror("sendto");
        } else {
            std::cout << "发送一帧: " << sent << " 字节\n";
        }

        if(cv::waitKey(1) == 'q') break;
    }

    close(sock);
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
