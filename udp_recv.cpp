#include <opencv2/opencv.hpp>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main() {
    // ------------------------
    //  创建 UDP 套接字
    // ------------------------
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    // ------------------------
    //  绑定接收端地址和端口
    // ------------------------
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);

    // 绑定所有网络接口（Docker 内部网络也会生效）
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    std::cout << "接收端监听 UDP 8888 端口，等待视频帧...\n";

    // 最大单包 UDP 大小（MTU 限制 <= 65507）
    std::vector<uchar> buffer(65535);

    while (true) {
        sockaddr_in senderAddr;
        socklen_t senderLen = sizeof(senderAddr);

        // ------------------------
        //  接收 UDP 数据
        // ------------------------
        int recvLen = recvfrom(sock,
                               buffer.data(),
                               buffer.size(),
                               0,
                               (sockaddr*)&senderAddr,
                               &senderLen);

        if (recvLen <= 0) {
            std::cerr << "接收失败\n";
            continue;
        }

        // ------------------------
        //  将收到的数据解码成 JPEG
        // ------------------------
        std::vector<uchar> jpg(buffer.begin(), buffer.begin() + recvLen);
        cv::Mat frame = cv::imdecode(jpg, cv::IMREAD_COLOR);

        if (frame.empty()) {
            std::cerr << "JPEG 解码失败\n";
            continue;
        }

        // ------------------------
        //  显示画面
        // ------------------------
        cv::imshow("UDP Receiver", frame);
        if (cv::waitKey(1) == 'q') break;

        std::cout << "收到帧: " << recvLen << " 字节\n";
    }

    close(sock);
    cv::destroyAllWindows();
    return 0;
}
