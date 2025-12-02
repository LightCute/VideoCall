#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *v4l2src, *capsfilter;
    GstElement *tee, *queue_display, *queue_network;
    GstElement *jpegdec, *videoconvert, *autovideosink;
    GstElement *rtpjpegpay, *udpsink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    // 创建元素
    v4l2src       = gst_element_factory_make("v4l2src", "v4l2src");
    capsfilter    = gst_element_factory_make("capsfilter", "capsfilter");
    tee           = gst_element_factory_make("tee", "tee");
    queue_display = gst_element_factory_make("queue", "queue_display");
    queue_network = gst_element_factory_make("queue", "queue_network");
    jpegdec       = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert  = gst_element_factory_make("videoconvert", "videoconvert");
    autovideosink = gst_element_factory_make("xvimagesink", "autovideosink");
    rtpjpegpay    = gst_element_factory_make("rtpjpegpay", "rtpjpegpay");
    udpsink       = gst_element_factory_make("udpsink", "udpsink");

    if (!v4l2src || !capsfilter || !tee || !queue_display || !queue_network ||
        !jpegdec || !videoconvert || !autovideosink || !rtpjpegpay || !udpsink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // 创建 pipeline
    pipeline = gst_pipeline_new("mjpeg-rtp-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create pipeline.\n");
        return -1;
    }

    // 配置 v4l2src
    g_object_set(v4l2src, "device", "/dev/video4", NULL);

    // 配置 caps: MJPEG 640x480 @30fps
    GstCaps *caps = gst_caps_new_simple(
        "image/jpeg",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    // 配置 udpsink
    g_object_set(udpsink,
                 "host", "10.0.0.4",
                 "port", 5000,
                 "sync", FALSE,
                 "async", FALSE,
                 NULL);

    // 队列配置低延迟
    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_network, "max-size-buffers", 1, "leaky", 2, NULL);

    // 配置本地显示
    g_object_set(autovideosink, "sync", FALSE, NULL);

    // 配置 RTP MJPEG
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);  // 分包大小

    // 添加元素到 pipeline
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, jpegdec, videoconvert, autovideosink,
                     queue_network, rtpjpegpay, udpsink,
                     NULL);

    // 链接 source -> caps -> tee
    if (!gst_element_link_many(v4l2src, capsfilter, tee, NULL)) {
        g_printerr("Failed to link source -> caps -> tee.\n");
        return -1;
    }

    // tee -> display 分支
    GstPad *tee_pad_display = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_display_sink = gst_element_get_static_pad(queue_display, "sink");
    gst_pad_link(tee_pad_display, queue_display_sink);
    gst_object_unref(queue_display_sink);
    if (!gst_element_link_many(queue_display, jpegdec, videoconvert, autovideosink, NULL)) {
        g_printerr("Failed to link display branch.\n");
        return -1;
    }

    // tee -> network 分支
    GstPad *tee_pad_network = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_network_sink = gst_element_get_static_pad(queue_network, "sink");
    gst_pad_link(tee_pad_network, queue_network_sink);
    gst_object_unref(queue_network_sink);
    if (!gst_element_link_many(queue_network, rtpjpegpay, udpsink, NULL)) {
        g_printerr("Failed to link network branch.\n");
        return -1;
    }

    // 设置 pipeline 运行
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set pipeline to PLAYING.\n");
        return -1;
    }

    // 等待错误或 EOS
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *debug_info;
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("GStreamer error: %s\n", err->message);
            g_error_free(err);
            g_free(debug_info);
        } else {
            g_print("End-Of-Stream reached.\n");
        }
        gst_message_unref(msg);
    }

    // 清理
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_element_release_request_pad(tee, tee_pad_display);
    gst_element_release_request_pad(tee, tee_pad_network);
    gst_object_unref(tee_pad_display);
    gst_object_unref(tee_pad_network);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
