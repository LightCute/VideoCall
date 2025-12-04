// gst_sender_h264_fixed.cpp
#include <gst/gst.h>
#include <glib.h>
#include <stdint.h>

typedef struct {
    guint64 total_bytes;
    guint64 last_time_s;
} Stats;

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
        g_print("Sender: frame=%zu bytes, bandwidth=%.2f KB/s\n", size, kbps);
        stats->total_bytes = 0;
        stats->last_time_s = now_s;
    }

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[])
{
    gst_init(&argc, &argv);

    GstElement *pipeline = NULL;
    GstElement *src = NULL, *capsfilter = NULL, *tee = NULL;
    GstElement *queue_display = NULL, *h264parse_disp = NULL, *avdec = NULL, *videoconvert = NULL, *videosink = NULL;
    GstElement *queue_net = NULL, *h264parse_net = NULL, *rtppay = NULL, *udpsink = NULL;
    GstBus *bus = NULL;
    GstMessage *msg = NULL;

    /* create elements */
    src            = gst_element_factory_make("v4l2src", "src");
    capsfilter     = gst_element_factory_make("capsfilter", "capsfilter");
    tee            = gst_element_factory_make("tee", "tee");

    queue_display  = gst_element_factory_make("queue", "q_disp");
    h264parse_disp = gst_element_factory_make("h264parse", "h264parse_disp");
    avdec          = gst_element_factory_make("avdec_h264", "avdec");
    videoconvert   = gst_element_factory_make("videoconvert", "videoconvert_disp");
    videosink      = gst_element_factory_make("xvimagesink", "videosink");

    queue_net      = gst_element_factory_make("queue", "q_net");
    h264parse_net  = gst_element_factory_make("h264parse", "h264parse_net");
    rtppay         = gst_element_factory_make("rtph264pay", "rtppay");
    udpsink        = gst_element_factory_make("udpsink", "udpsink");

    if (!src || !capsfilter || !tee ||
        !queue_display || !h264parse_disp || !avdec || !videoconvert || !videosink ||
        !queue_net || !h264parse_net || !rtppay || !udpsink) {
        g_printerr("Failed to create GStreamer elements.\n");
        return -1;
    }

    pipeline = gst_pipeline_new("h264_cam_rtp");
    if (!pipeline) {
        g_printerr("Failed to create pipeline\n");
        return -1;
    }

    /* set device (确认 /dev/video2 真的是 H264 输出设备) */
    g_object_set(src, "device", "/dev/video2", NULL);

    /* caps: 只声明 H.264 的 stream-format / alignment，不要强制 width/height/framerate */
    GstCaps *caps = gst_caps_new_simple(
        "video/x-h264",
        "stream-format", G_TYPE_STRING, "byte-stream",
        "alignment",     G_TYPE_STRING, "au",
        // "width", G_TYPE_INT, 640, 
        // "height", G_TYPE_INT, 480, 
        // "framerate", GST_TYPE_FRACTION, 30, 1,         
        NULL
    );
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    /* low-latency queues and sink settings */
    g_object_set(queue_display, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue_net,     "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(videosink, "sync", FALSE, NULL);

    /* rtppay and udpsink */
    g_object_set(rtppay, "pt", 96, "config-interval", 1, NULL);
    g_object_set(udpsink,
                 "host", "10.0.0.4",
                 "port", 5000,
                 "sync", FALSE,
                 "async", FALSE,
                 NULL);

    /* add to pipeline */
    gst_bin_add_many(GST_BIN(pipeline),
                     src, capsfilter, tee,
                     queue_display, h264parse_disp, avdec, videoconvert, videosink,
                     queue_net, h264parse_net, rtppay, udpsink,
                     NULL);

    /* link src -> caps -> tee */
    if (!gst_element_link_many(src, capsfilter, tee, NULL)) {
        g_printerr("Failed link: src -> caps -> tee\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* display branch */
    GstPad *tee_pad_disp = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *q_disp_sink = gst_element_get_static_pad(queue_display, "sink");
    if (!q_disp_sink) {
        g_printerr("Couldn't get queue_display sink pad\n");
        gst_object_unref(pipeline);
        return -1;
    }
    if (gst_pad_link(tee_pad_disp, q_disp_sink) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee -> queue_display pad\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_object_unref(q_disp_sink);

    if (!gst_element_link_many(queue_display, h264parse_disp, avdec, videoconvert, videosink, NULL)) {
        g_printerr("Failed to link display branch\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* network branch */
    GstPad *tee_pad_net = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *q_net_sink = gst_element_get_static_pad(queue_net, "sink");
    if (!q_net_sink) {
        g_printerr("Couldn't get queue_net sink pad\n");
        gst_object_unref(pipeline);
        return -1;
    }
    if (gst_pad_link(tee_pad_net, q_net_sink) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee -> queue_net pad\n");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_object_unref(q_net_sink);

    if (!gst_element_link_many(queue_net, h264parse_net, rtppay, udpsink, NULL)) {
        g_printerr("Failed to link network branch\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* bandwidth probe: place on rtppay src pad (RTP packets about to go out) */
    GstPad *probe_pad = gst_element_get_static_pad(rtppay, "src");
    if (probe_pad) {
        Stats stats = {0, 0};
        gst_pad_add_probe(probe_pad, GST_PAD_PROBE_TYPE_BUFFER, bandwidth_probe, &stats, NULL);
        gst_object_unref(probe_pad);
    } else {
        g_printerr("Warning: couldn't get rtppay src pad for probe\n");
    }

    /* start */
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* wait for error or EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (msg) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *dbg;
            gst_message_parse_error(msg, &err, &dbg);
            g_printerr("Error from pipeline: %s\n", err->message);
            g_error_free(err);
            g_free(dbg);
        } else {
            g_print("End-Of-Stream reached.\n");
        }
        gst_message_unref(msg);
    }

    /* cleanup */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
