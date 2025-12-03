#include <gst/gst.h>
#include <glib.h>
#include <stdint.h>

typedef struct {
    guint64 total_bytes;
    guint64 last_time_s;
} Stats;

/* ---- 带宽统计 (可选) ---- */
static GstPadProbeReturn bandwidth_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    Stats *stats = (Stats*)user_data;
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    gsize size = gst_buffer_get_size(buf);
    stats->total_bytes += size;

    guint64 now_s = g_get_real_time() / G_USEC_PER_SEC;
    if (now_s != stats->last_time_s) {
        double kbps = (double)stats->total_bytes / 1024.0;
        g_print("Receiver: bandwidth = %.2f KB/s\n", kbps);
        stats->total_bytes = 0;
        stats->last_time_s = now_s;
    }

    return GST_PAD_PROBE_OK;
}


int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    GstElement *pipeline = NULL;
    GstElement *udpsrc = NULL;
    GstElement *rtph264depay = NULL;
    GstElement *h264parse = NULL;
    GstElement *avdec = NULL;
    GstElement *videoconvert = NULL;
    GstElement *videosink = NULL;

    GstBus *bus = NULL;
    GstMessage *msg = NULL;

    /* Create elements */
    udpsrc        = gst_element_factory_make("udpsrc", "udpsrc");
    rtph264depay  = gst_element_factory_make("rtph264depay", "rtph264depay");
    h264parse     = gst_element_factory_make("h264parse", "h264parse");
    avdec         = gst_element_factory_make("avdec_h264", "avdec");
    videoconvert  = gst_element_factory_make("videoconvert", "videoconvert");
    videosink     = gst_element_factory_make("autovideosink", "videosink");

    if (!udpsrc || !rtph264depay || !h264parse || !avdec || !videoconvert || !videosink) {
        g_printerr("Failed to create receiver elements.\n");
        return -1;
    }

    pipeline = gst_pipeline_new("receiver_pipeline");

    /* 设置 UDP 端口 */
    g_object_set(udpsrc, "port", 5000, NULL);

    /* 设置 RTP caps（必须与发送端匹配） */
    GstCaps *caps = gst_caps_new_simple(
        "application/x-rtp",
        "media", G_TYPE_STRING, "video",
        "encoding-name", G_TYPE_STRING, "H264",
        "clock-rate", G_TYPE_INT, 90000,
        "payload", G_TYPE_INT, 96,        /* PT=96 */
        NULL
    );
    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);

    /* 把元素加入 pipeline */
    gst_bin_add_many(GST_BIN(pipeline),
                     udpsrc, rtph264depay, h264parse,
                     avdec, videoconvert, videosink,
                     NULL);

    /* 链接 */
    if (!gst_element_link_many(rtph264depay, h264parse, avdec, videoconvert, videosink, NULL)) {
        g_printerr("Failed to link receiver pipeline.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* udpsrc 需要手工链接到 depay，因为 udpsrc 有 src pad */
    GstPad *srcpad = gst_element_get_static_pad(udpsrc, "src");
    GstPad *sinkpad = gst_element_get_static_pad(rtph264depay, "sink");
    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link udpsrc → rtph264depay.\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_object_unref(srcpad);
    gst_object_unref(sinkpad);

    /* probe（可选） */
    GstPad *probe_pad = gst_element_get_static_pad(rtph264depay, "src");
    if (probe_pad) {
        Stats stats = {0, 0};
        gst_pad_add_probe(probe_pad, GST_PAD_PROBE_TYPE_BUFFER,
                          bandwidth_probe, &stats, NULL);
        gst_object_unref(probe_pad);
    }

    /* 运行 pipeline */
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Receiver pipeline cannot PLAY.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* 等待错误或 EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            g_printerr("Receiver Error: %s\n", err->message);
            g_error_free(err);
            g_free(dbg);
        } else {
            g_print("Receiver: EOS\n");
        }
        gst_message_unref(msg);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);
    return 0;
}
