#include <gst/gst.h>
#include <gst/video/video.h>
#include <string.h>

typedef struct {
    guint64 total_bytes;
    guint64 last_time;
} Stats;

// pad probe：添加 8 字节时间戳 + 统计帧大小
static GstPadProbeReturn pad_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Stats *stats = (Stats *)user_data;
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    //--------------------------
    // 1. 生成时间戳（微秒级）
    //--------------------------
    guint64 ts = g_get_real_time();  // 获取当前时间（us）

    //-----------------------------------
    // 2. 创建一个 8 字节 memory 放时间戳
    //-----------------------------------
    GstMemory *ts_mem = gst_memory_new_wrapped(
        GST_MEMORY_FLAG_READONLY,
        g_memdup(&ts, sizeof(ts)),
        sizeof(ts),
        0,
        sizeof(ts),
        NULL,
        g_free);

    //--------------------------
    // 3. 把 timestamp 放到 buffer 最前面
    //--------------------------
    buf = gst_buffer_make_writable(buf);
    gst_buffer_prepend_memory(buf, ts_mem);

    //-----------------------------------
    // 4. 统计带宽
    //-----------------------------------
    gsize size = gst_buffer_get_size(buf);
    stats->total_bytes += size;

    guint64 now = g_get_real_time() / 1000000;
    if (now != stats->last_time) {
        g_print("Frame size: %zu bytes, bandwidth: %.2f KB/s\n",
                size, stats->total_bytes / 1024.0);
        stats->total_bytes = 0;
        stats->last_time = now;
    }

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[])
{
    GstElement *pipeline, *v4l2src, *capsfilter;
    GstElement *tee, *queue_display, *queue_network;
    GstElement *jpegenc, *videoconvert, *autovideosink;
    GstElement *rtpjpegpay, *udpsink;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    //-----------------------
    // 创建元素
    //-----------------------
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

    pipeline = gst_pipeline_new("mjpeg-rtp-pipeline");

    //-----------------------
    // 配置参数
    //-----------------------
    g_object_set(v4l2src, "device", "/dev/video0", NULL);

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT, 480,
                                        "framerate", GST_TYPE_FRACTION, 30, 1,
                                        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    g_object_set(jpegenc, "quality", 50, NULL);

    g_object_set(udpsink,
                 "host", "10.0.0.4",
                 "port", 5000,
                 "sync", FALSE,
                 "async", FALSE,
                 NULL);

    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_network, "max-size-buffers", 1, "leaky", 2, NULL);

    g_object_set(autovideosink, "sync", FALSE, NULL);
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    //-----------------------
    // 连接元素
    //-----------------------
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue_display, videoconvert, autovideosink,
                     queue_network, jpegenc, rtpjpegpay, udpsink,
                     NULL);

    gst_element_link_many(v4l2src, capsfilter, tee, NULL);

    GstPad *tee_pad_display = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *display_sink = gst_element_get_static_pad(queue_display, "sink");
    gst_pad_link(tee_pad_display, display_sink);
    gst_object_unref(display_sink);

    gst_element_link_many(queue_display, videoconvert, autovideosink, NULL);

    GstPad *tee_pad_network = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *network_sink = gst_element_get_static_pad(queue_network, "sink");
    gst_pad_link(tee_pad_network, network_sink);
    gst_object_unref(network_sink);

    gst_element_link_many(queue_network, jpegenc, rtpjpegpay, udpsink, NULL);

    //-----------------------
    // 添加 pad probe（写入时间戳）
    //-----------------------
    GstPad *src_pad = gst_element_get_static_pad(jpegenc, "src");
    Stats stats = {0, 0};
    gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, pad_probe_callback, &stats, NULL);
    gst_object_unref(src_pad);

    //-----------------------
    // 运行
    //-----------------------
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg)
        gst_message_unref(msg);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
