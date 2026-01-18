// gst_sender_fixed.c
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
    g_object_set(v4l2src, "device", "/dev/video0", NULL);

    // 配置 caps: MJPEG from camera 640x480@30
    GstCaps *caps = gst_caps_new_simple(
        "image/jpeg",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        "quality", G_TYPE_INT, 50,    // ⭐ 降低 JPEG 质量
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

    // 队列低延迟
    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_network, "max-size-buffers", 1, "leaky", 2, NULL);

    // 本地显示不等待
    g_object_set(autovideosink, "sync", FALSE, NULL);

    // RTP 分包
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    // 添加元素（注意顺序不影响，但元素必须都 add 到 pipeline）
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, jpegdec, videoconvert, autovideosink,
                     queue_network, rtpjpegpay, udpsink,
                     NULL);

    // 链接 source -> caps -> tee
    if (!gst_element_link_many(v4l2src, capsfilter, tee, NULL)) {
        g_printerr("Failed to link source -> caps -> tee.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // tee -> display 分支: queue_display -> jpegdec -> videoconvert -> sink
    GstPad *tee_pad_display = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_display_sink = gst_element_get_static_pad(queue_display, "sink");
    if (gst_pad_link(tee_pad_display, queue_display_sink) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee -> queue_display pad.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_object_unref(queue_display_sink);

    if (!gst_element_link_many(queue_display, jpegdec, videoconvert, autovideosink, NULL)) {
        g_printerr("Failed to link display branch.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // tee -> network 分支: queue_network -> rtpjpegpay -> udpsink
    GstPad *tee_pad_network = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_network_sink = gst_element_get_static_pad(queue_network, "sink");
    if (gst_pad_link(tee_pad_network, queue_network_sink) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee -> queue_network pad.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_object_unref(queue_network_sink);

    if (!gst_element_link_many(queue_network, rtpjpegpay, udpsink, NULL)) {
        g_printerr("Failed to link network branch.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // 在 rtpjpegpay 的 src pad 前可以统计帧大小：找 jpeg 原始数据前的 pad 或直接在 queue_network 后 probe。
    GstPad *probe_pad = gst_element_get_static_pad(queue_network, "src");
    Stats stats = {0, 0};
    gst_pad_add_probe(probe_pad, GST_PAD_PROBE_TYPE_BUFFER, bandwidth_probe, &stats, NULL);
    gst_object_unref(probe_pad);

    // 运行 pipeline
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set pipeline to PLAYING.\n");
        gst_object_unref(pipeline);
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
