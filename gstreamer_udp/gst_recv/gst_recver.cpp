#include <gst/gst.h>
#include <string.h>

// pad probe：取出写入的 8 字节时间戳并计算延迟
static GstPadProbeReturn pad_probe_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    GstBuffer *buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    //---------------------------------------
    // 读取前 8 字节的 timestamp
    //---------------------------------------
    if (gst_buffer_get_size(buf) < 8)
        return GST_PAD_PROBE_OK;

    GstMapInfo map;
    gst_buffer_map(buf, &map, GST_MAP_READ);

    guint64 ts = 0;
    memcpy(&ts, map.data, sizeof(ts));   // 提取 timestamp

    gst_buffer_unmap(buf, &map);

    //---------------------------------------
    // 计算延迟
    //---------------------------------------
    guint64 now = g_get_real_time();
    double latency_ms = (now - ts) / 1000.0;

    g_print("Latency = %.3f ms (frame size: %zu bytes)\n",
            latency_ms, gst_buffer_get_size(buf));

    //---------------------------------------
    // 必须从 buffer 中移除前 8 字节 timestamp
    //---------------------------------------
    buf = gst_buffer_make_writable(buf);
    gst_buffer_resize(buf, sizeof(ts), -sizeof(ts));

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[])
{
    GstElement *pipeline, *udpsrc, *rtpjpegdepay, *jpegparse, *jpegdec, *videoconvert, *sink;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    udpsrc       = gst_element_factory_make("udpsrc", "udpsrc");
    rtpjpegdepay = gst_element_factory_make("rtpjpegdepay", "rtpjpegdepay");
    jpegparse    = gst_element_factory_make("jpegparse", "jpegparse");
    jpegdec      = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    sink         = gst_element_factory_make("xvimagesink", "sink");

    pipeline = gst_pipeline_new("recv");

    g_object_set(udpsrc, "port", 5000, NULL);

    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
                                        "media", G_TYPE_STRING, "video",
                                        "encoding-name", G_TYPE_STRING, "JPEG",
                                        "payload", G_TYPE_INT, 26,
                                        NULL);
    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN(pipeline),
                     udpsrc, rtpjpegdepay, jpegparse,
                     jpegdec, videoconvert, sink, NULL);

    gst_element_link_many(udpsrc, rtpjpegdepay, jpegparse,
                          jpegdec, videoconvert, sink, NULL);

    //---------------------------------------
    // pad probe 放在 rtpjpegdepay 的 src pad
    //---------------------------------------
    GstPad *probe_pad = gst_element_get_static_pad(rtpjpegdepay, "src");
    gst_pad_add_probe(probe_pad, GST_PAD_PROBE_TYPE_BUFFER, pad_probe_callback, NULL, NULL);
    gst_object_unref(probe_pad);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus,
                                     GST_CLOCK_TIME_NONE,
                                     GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (msg)
        gst_message_unref(msg);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
