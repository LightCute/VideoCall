#include <iostream>
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);  // 初始化 GStreamer 库

    // 创建发送端管道：采集视频并通过 UDP 发送，同时显示视频
    // std::string pipeline_str = "v4l2src device=/dev/video0 ! videoconvert ! 'video/x-raw,format=(string)NV12,width=640,height=480,framerate=(fraction)30/1' ! tee name=t ! "
    //                            "queue ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay ! udpsink host=172.18.0.3 port=5000 sync=true "
    //                            "t. ! queue ! autovideosink";

    std::string pipeline_str = "v4l2src device=/dev/video0 ! videoconvert ! 'video/x-raw,format=(string)NV12,width=640,height=480,framerate=(fraction)30/1' ! tee name=t ! "
                               //"queue ! x264enc tune=zerolatency speed-preset=ultrafast ! rtph264pay ! udpsink host=172.18.0.3 port=5000 sync=true "
                               " queue ! autovideosink";


    // 创建管道
    GstElement *pipeline = gst_parse_launch(pipeline_str.c_str(), NULL);
    if (!pipeline) {
        std::cerr << "无法创建 GStreamer 管道" << std::endl;
        return -1;
    }

    // 获取管道的总线，用于处理消息
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg;

    // 设置管道状态为播放
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    std::cout << "管道设置为 PLAYING 状态" << std::endl;

    // 等待 GStreamer 错误或流结束消息
    while (true) {
        // 使用按位或操作符来组合多个事件，并确保传递的是 GstMessageType 类型
        GstMessageType events = static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
        msg = gst_bus_poll(bus, events, -1);
        if (msg) {
            // 检查消息类型
            if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                GError *err;
                gchar *debug_info;
                gst_message_parse_error(msg, &err, &debug_info);
                std::cerr << "GStreamer 错误: " << err->message << std::endl;
                g_error_free(err);
                g_free(debug_info);
                break;
            } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                std::cout << "流结束" << std::endl;
                break;
            }
        }
    }

    // 清理资源
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
