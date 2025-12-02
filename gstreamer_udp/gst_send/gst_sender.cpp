// sender.c
#include <gst/gst.h>
#include <glib.h>
#include <string.h>
#include <stdint.h>

/*
 * 发送端：
 * - 在 jpegenc 的 src pad 上为每帧 prepend 一个 8 字节的 big-endian 时间戳 (microseconds)
 * - 统计每秒发送字节数（包含 timestamp）
 */

typedef struct {
    guint64 total_bytes;
    guint64 last_time_s;
} Stats;

static guint64 host_to_be_u64(guint64 v) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    return GINT64_TO_BE((gint64)v);
#else
    return v;
#endif
}

static GstPadProbeReturn sender_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    Stats *stats = (Stats *)user_data;
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    // 获取当前时间（microseconds）
    guint64 now_us = g_get_real_time();

    // 转换为 big-endian
    guint64 ts_be = host_to_be_u64(now_us);

    // 包装 time memory (会被 gst 释放，使用 g_memdup 并传入 g_free)
    gpointer memcopy = g_memdup(&ts_be, sizeof(ts_be));
    GstMemory *ts_mem = gst_memory_new_wrapped(
        GST_MEMORY_FLAG_READONLY,
        memcopy,                // data pointer (owned by gst memory, will be freed by g_free)
        sizeof(ts_be),         // size
        0,                     // offset
        sizeof(ts_be),         // maxsize (we used full)
        NULL,
        g_free
    );

    // make writable and prepend timestamp memory
    buf = gst_buffer_make_writable(buf);
    gst_buffer_prepend_memory(buf, ts_mem);

    // 统计带宽（每秒）
    gsize size = gst_buffer_get_size(buf);
    stats->total_bytes += size;
    guint64 now_s = g_get_real_time() / 1000000ULL;
    if (now_s != stats->last_time_s) {
        g_print("Sender: Frame size: %zu bytes, bandwidth: %.2f KB/s\n",
                size, (double)stats->total_bytes / 1024.0);
        stats->total_bytes = 0;
        stats->last_time_s = now_s;
    }

    // 注意：我们修改了 buffer（prepend），probe 返回后会继续流到 rtpjpegpay -> udpsink
    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstElement *pipeline, *v4l2src, *capsfilter, *tee;
    GstElement *queue_display, *queue_network, *jpegenc, *videoconvert, *autovideosink;
    GstElement *rtpjpegpay, *udpsink;
    GstBus *bus;
    GstMessage *msg;

    pipeline = gst_pipeline_new("mjpeg-rtp-pipeline-sender");

    v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    tee = gst_element_factory_make("tee", "tee");
    queue_display = gst_element_factory_make("queue", "queue_display");
    queue_network = gst_element_factory_make("queue", "queue_network");
    jpegenc = gst_element_factory_make("jpegenc", "jpegenc");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    autovideosink = gst_element_factory_make("xvimagesink", "autovideosink");
    rtpjpegpay = gst_element_factory_make("rtpjpegpay", "rtpjpegpay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");

    if (!pipeline || !v4l2src || !capsfilter || !tee || !queue_display || !queue_network ||
        !jpegenc || !videoconvert || !autovideosink || !rtpjpegpay || !udpsink) {
        g_printerr("Sender: Not all elements could be created.\n");
        return -1;
    }

    // 配置
    g_object_set(v4l2src, "device", "/dev/video0", NULL);

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(jpegenc, "quality", 50, NULL);

    g_object_set(udpsink, "host", "10.0.0.4", "port", 5000, "sync", FALSE, "async", FALSE, NULL);

    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_network, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(autovideosink, "sync", FALSE, NULL);
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    // add and link
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, videoconvert, autovideosink,
                     queue_network, jpegenc, rtpjpegpay, udpsink,
                     NULL);

    if (!gst_element_link_many(v4l2src, capsfilter, tee, NULL)) {
        g_printerr("Sender: failed link source->caps->tee\n");
        return -1;
    }

    // display branch
    GstPad *tee_pad_display = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_display_sink = gst_element_get_static_pad(queue_display, "sink");
    gst_pad_link(tee_pad_display, queue_display_sink);
    gst_object_unref(queue_display_sink);
    if (!gst_element_link_many(queue_display, videoconvert, autovideosink, NULL)) {
        g_printerr("Sender: failed link display branch\n");
        return -1;
    }

    // network branch
    GstPad *tee_pad_network = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue_network_sink = gst_element_get_static_pad(queue_network, "sink");
    gst_pad_link(tee_pad_network, queue_network_sink);
    gst_object_unref(queue_network_sink);
    if (!gst_element_link_many(queue_network, jpegenc, rtpjpegpay, udpsink, NULL)) {
        g_printerr("Sender: failed link network branch\n");
        return -1;
    }

    // 在 jpegenc src pad 添加 probe（写入 timestamp）
    GstPad *src_pad = gst_element_get_static_pad(jpegenc, "src");
    Stats stats = {0, 0};
    gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, sender_probe, &stats, NULL);
    gst_object_unref(src_pad);

    // run
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (msg) gst_message_unref(msg);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
