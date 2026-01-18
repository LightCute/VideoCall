#include <gst/gst.h>
#include <glib.h>
#include <stdint.h>

typedef struct {
    guint64 total_bytes;
    guint64 last_time_s;
} Stats;

/* pad probe：统计每帧大小并每秒打印带宽 */
static GstPadProbeReturn bandwidth_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Stats *stats = (Stats *)user_data;
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    /* 当前帧字节数 */
    gsize size = gst_buffer_get_size(buf);
    stats->total_bytes += size;

    /* 每秒打印带宽（KB/s）并清零累计 */
    guint64 now_s = g_get_real_time() / 1000000ULL; /* seconds */
    if (now_s != stats->last_time_s) {
        double kb_per_s = (double)stats->total_bytes / 1024.0;
        g_print("Sender: Frame size: %zu bytes, bandwidth: %.2f KB/s\n", size, kb_per_s);
        stats->total_bytes = 0;
        stats->last_time_s = now_s;
    }

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[]) {
    GstElement *pipeline, *v4l2src, *capsfilter;
    GstElement *tee, *queue_display, *queue_network, *queue_receive;
    GstElement *jpegenc, *videoconvert, *autovideosink;
    GstElement *rtpjpegpay, *udpsink;
    GstElement *udpsrc_receive, *rtpjpegdepay_receive, *jpegparse_receive, *jpegdec_receive, *videoconvert_receive, *videosink_receive;
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
    queue_receive = gst_element_factory_make("queue", "queue_receive");
    jpegenc       = gst_element_factory_make("jpegenc", "jpegenc");
    videoconvert  = gst_element_factory_make("videoconvert", "videoconvert");
    autovideosink = gst_element_factory_make("xvimagesink", "autovideosink");
    rtpjpegpay    = gst_element_factory_make("rtpjpegpay", "rtpjpegpay");
    udpsink       = gst_element_factory_make("udpsink", "udpsink");

    // 接收端元素
    udpsrc_receive       = gst_element_factory_make("udpsrc", "udpsrc_receive");
    rtpjpegdepay_receive = gst_element_factory_make("rtpjpegdepay", "rtpjpegdepay_receive");
    jpegparse_receive    = gst_element_factory_make("jpegparse", "jpegparse_receive");
    jpegdec_receive      = gst_element_factory_make("jpegdec", "jpegdec_receive");
    videoconvert_receive = gst_element_factory_make("videoconvert", "videoconvert_receive");
    videosink_receive    = gst_element_factory_make("xvimagesink", "videosink_receive");

    if (!v4l2src || !capsfilter || !tee || !queue_display || !queue_network ||
        !jpegenc || !videoconvert || !autovideosink || !rtpjpegpay || !udpsink ||
        !udpsrc_receive || !rtpjpegdepay_receive || !jpegparse_receive || !jpegdec_receive ||
        !videoconvert_receive || !videosink_receive) {
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
    g_object_set(v4l2src, "device", "/dev/video0", NULL);

    // 配置 caps: 640x480 @30fps
    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    // 配置 jpegenc 压缩质量
    g_object_set(jpegenc, "quality", 50, NULL); // 0~100, 越低帧越小

    // 配置 udpsink
    g_object_set(udpsink,
                 "host", "10.0.0.4",
                 "port", 5000,
                 "sync", FALSE,
                 "async", FALSE,
                 NULL);

    // 队列低延迟
    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_network, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_receive, "max-size-buffers", 1, "leaky", 2, NULL);

    // 本地显示不等待
    g_object_set(autovideosink, "sync", FALSE, NULL);

    // RTP 分包
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    // 配置接收端的 udpsrc
    g_object_set(udpsrc_receive, "port", 5001, NULL);
    GstCaps *caps_receive = gst_caps_new_simple("application/x-rtp",
        "media", G_TYPE_STRING, "video",
        "encoding-name", G_TYPE_STRING, "JPEG",
        "payload", G_TYPE_INT, 26,
        NULL);
    g_object_set(udpsrc_receive, "caps", caps_receive, NULL);
    gst_caps_unref(caps_receive);

    // GstPad *srcpad_receive, *sinkpad_receive;
    // GstCaps *caps_receive;

    // // 获取 udpsrc 的 src pad
    // srcpad_receive = gst_element_get_static_pad(udpsrc_receive, "src");
    // // 获取 queue 的 sink pad
    // sinkpad_receive = gst_element_get_static_pad(queue_receive, "sink");

    // // 创建 caps 以明确指定格式
    // caps_receive = gst_caps_new_simple("application/x-rtp", NULL);
    // gst_pad_set_caps(srcpad_receive, caps_receive);

    // // 尝试连接
    // if (gst_pad_link(srcpad_receive, sinkpad_receive) != GST_PAD_LINK_OK) {
    //     g_print("Failed to link udpsrc_receive -> queue_receive\n");
    // } else {
    //     g_print("udpsrc_receive -> queue_receive linked successfully\n");
    // }

    // // 释放资源
    // gst_caps_unref(caps_receive);
    // gst_object_unref(srcpad_receive);
    // gst_object_unref(sinkpad_receive);


    // 添加元素
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, videoconvert, autovideosink,
                     queue_network, jpegenc, rtpjpegpay, udpsink,
                     udpsrc_receive, rtpjpegdepay_receive, jpegparse_receive, jpegdec_receive, videoconvert_receive, videosink_receive,
                     NULL);

    // 链接 source -> caps -> tee
    if (!gst_element_link_many(v4l2src, capsfilter, tee, NULL)) {
        g_printerr("Failed to link source -> caps -> tee.\n");
        return -1;
    }

    // tee -> display 分支: queue_display -> videoconvert -> sink
    GstPad *tee_pad_display = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_display_sink = gst_element_get_static_pad(queue_display, "sink");
    gst_pad_link(tee_pad_display, queue_display_sink);
    gst_object_unref(queue_display_sink);
    if (!gst_element_link_many(queue_display, videoconvert, autovideosink, NULL)) {
        g_printerr("Failed to link display branch.\n");
        return -1;
    }


    // tee -> network 分支: queue_network -> jpegenc -> rtpjpegpay -> udpsink
    GstPad *tee_pad_network = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_network_sink = gst_element_get_static_pad(queue_network, "sink");
    gst_pad_link(tee_pad_network, queue_network_sink);
    gst_object_unref(queue_network_sink);
    if (!gst_element_link_many(queue_network, jpegenc, rtpjpegpay, udpsink, NULL)) {
        g_printerr("Failed to link network branch.\n");
        return -1;
    }


    // // tee -> receive 分支: queue_receive -> udpsrc_receive -> rtpjpegdepay_receive -> jpegparse_receive -> jpegdec_receive -> videoconvert_receive -> videosink_receive
    if (!gst_element_link_many(queue_receive, udpsrc_receive, rtpjpegdepay_receive, jpegparse_receive, jpegdec_receive, videoconvert_receive, videosink_receive, NULL)) {
        g_printerr("Failed to link receive branch.\n");
        return -1;
    }


    // 在 jpegenc 的 src pad 添加带宽统计 probe（rtpjpegpay 前）
    GstPad *src_pad_cal_bandwidth = gst_element_get_static_pad(jpegenc, "src");
    Stats stats_cal_bandwidth = {0, 0};
    gst_pad_add_probe(src_pad_cal_bandwidth, GST_PAD_PROBE_TYPE_BUFFER, bandwidth_probe, &stats_cal_bandwidth, NULL);
    gst_object_unref(src_pad_cal_bandwidth);

    // 运行 pipeline
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
