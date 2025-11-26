#include <iostream>
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);  // 初始化 GStreamer 库

    // 创建 GStreamer 管道来显示摄像头视频
    std::string pipeline = "v4l2src device=/dev/video0 ! "
                           "video/x-raw,width=640,height=480,framerate=30/1 ! "
                           "videoconvert ! autovideosink";
    GstElement *pipelineElement = gst_parse_launch(pipeline.c_str(), NULL);

    if (!pipelineElement) {
        std::cerr << "无法创建 GStreamer 管道" << std::endl;
        return -1;
    }

    GstBus *bus = gst_element_get_bus(pipelineElement);
    GstMessage *msg;

    // 启动 GStreamer 管道
    gst_element_set_state(pipelineElement, GST_STATE_PLAYING);

    // 显示视频直到用户按下 'q' 键
    while (true) {
        msg = gst_bus_poll(bus, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS), 0);

        if (msg != NULL) {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            std::cerr << "GStreamer 错误: " << err->message << std::endl;
            g_error_free(err);
            g_free(debug);
            break;
        }
    }

    // 清理 GStreamer 资源
    gst_element_set_state(pipelineElement, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipelineElement);

    return 0;
}
