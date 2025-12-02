#include <gst/gst.h>
#include <gst/video/video.h>
#include <time.h>

typedef struct {
    guint64 total_bytes;
    guint64 last_time;
} Stats;

// pad probe 给每帧打时间戳 + 统计大小
static GstPadProbeReturn pad_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    Stats *stats = (Stats *)user_data;
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    // 发送端时间戳
    GST_BUFFER_PTS(buf) = gst_util_get_timestamp();

    gsize size = gst_buffer_get_size(buf);
    stats->total_bytes += size;

    // 每秒打印带宽
    guint64 now = g_get_real_time() / 1000000; // ms -> s
    if (now != stats->last_time) {
        g_print("Frame size: %zu bytes, bandwidth: %.2f KB/s\n",
                size, stats->total_bytes / 1024.0);
        stats->total_bytes = 0;
        stats->last_time = now;
    }

    return GST_PAD_PROBE_OK;
}



int main(int argc, char *argv[]) {
    GstElement *pipeline, *v4l2src, *capsfilter;
    GstElement *tee, *queue_display, *queue_network;
    GstElement *jpegenc, *videoconvert, *autovideosink;
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
    jpegenc       = gst_element_factory_make("jpegenc", "jpegenc");
    videoconvert  = gst_element_factory_make("videoconvert", "videoconvert");
    autovideosink = gst_element_factory_make("xvimagesink", "autovideosink");
    rtpjpegpay    = gst_element_factory_make("rtpjpegpay", "rtpjpegpay");
    udpsink       = gst_element_factory_make("udpsink", "udpsink");

    if (!v4l2src || !capsfilter || !tee || !queue_display || !queue_network ||
        !jpegenc || !videoconvert || !autovideosink || !rtpjpegpay || !udpsink) {
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

    // 本地显示不等待
    g_object_set(autovideosink, "sync", FALSE, NULL);

    // RTP 分包
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    // 添加元素
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, videoconvert, autovideosink,
                     queue_network, jpegenc, rtpjpegpay, udpsink,
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


    // pad probe 统计每帧大小
    GstPad *src_pad = gst_element_get_static_pad(jpegenc, "src");
    Stats stats = {0, 0};
    gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, pad_probe_callback, &stats, NULL);
    gst_object_unref(src_pad);

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
