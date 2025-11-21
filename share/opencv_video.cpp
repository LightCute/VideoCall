#include <opencv2/opencv.hpp>
#include <iostream>
#include <dlfcn.h>
#include <pthread.h>

int main() {
    // 打印出实际加载的 pthread 库路径
    Dl_info info;
    dladdr((void*)pthread_create, &info);
    std::cout << "pthread 来自: " << info.dli_fname << std::endl;

    // 打开摄像头
    // cv::VideoCapture cap(0);
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

cv::Mat frame;
while(true){
    if(!cap.read(frame) || frame.empty()){
        std::cerr << "Failed to grab frame" << std::endl;
        continue;
    }
    //cv::imshow("Camera", frame);
    std::cout << "读取到一帧，大小: " 
              << frame.cols << "x" << frame.rows << std::endl;
    if(cv::waitKey(30) == 'q') break;
}


    cap.release();
    cv::destroyAllWindows();
    return 0;
}
