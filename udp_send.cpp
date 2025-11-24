#include <opencv2/opencv.hpp>
#include <iostream>
#include <dlfcn.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main() {
    // 打印 pthread 库路径
    Dl_info info;
    dladdr((void*)pthread_create, &info);
    std::cout << "pthread 来自: " << info.dli_fname << std::endl;

    // 打开摄像头
    cv::VideoCapture cap("/dev/video0", cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    if(!cap.isOpened()) {
        std::cerr << "无法打开摄像头" << std::endl;
        return -1;
    }

    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "摄像头分辨率: " << width << "x" << height << std::endl;

    // ------------------------
    //  创建 UDP 套接字
    // ------------------------
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, "192.168.1.100", &addr.sin_addr); // 修改成接收端IP

    std::vector<uchar> jpgBuf; // JPEG 缓冲区

    cv::Mat frame;

    while(true) {
        if(!cap.read(frame) || frame.empty()) {
            std::cerr << "Failed to grab frame" << std::endl;
            continue;
        }

        // 显示视频
        cv::imshow("Camera", frame);

        // ------------------------
        //  压缩成 JPEG
        // ------------------------
        jpgBuf.clear();
        cv::imencode(".jpg", frame, jpgBuf);

        // ------------------------
        //  通过 UDP 发送
        // ------------------------
        sendto(sock,
               jpgBuf.data(),
               jpgBuf.size(),
               0,
               (sockaddr*)&addr,
               sizeof(addr));

        std::cout << "发送一帧: " << jpgBuf.size() << " 字节\n";

        if(cv::waitKey(1) == 'q') break;
    }

    close(sock);
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
