// receiver.c
#include <gst/gst.h>
#include <glib.h>
#include <string.h>
#include <stdint.h>

/*
 * 接收端：
 * - 在 rtpjpegdepay 的 src pad 上读取前 8 字节（big-endian timestamp，microseconds）
 * - 计算延迟（当前 g_get_real_time() - send_ts）
 * - 创建新的 GstBuffer（去掉 8 字节前缀）并替换原 buffer，继续 jpegparse/jdec
 *
 * 注意：发送端必须写入 big-endian 64-bit microseconds 前缀。
 */

// BE <-> host helpers
static guint64 be_to_host_u64(guint64 v) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    return GINT64_FROM_BE((gint64)v);
#else
    return v;
#endif
}
static GstPadProbeReturn recv_probe_strip_ts(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstBuffer *orig = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!orig) return GST_PAD_PROBE_OK;

    gsize orig_size = gst_buffer_get_size(orig);
    if (orig_size <= sizeof(guint64)) {
        g_print("Receiver probe: payload too small (%zu)\n", orig_size);
        return GST_PAD_PROBE_OK;
    }

    GstMapInfo map;
    if (!gst_buffer_map(orig, &map, GST_MAP_READ)) {
        return GST_PAD_PROBE_OK;
    }

    // --- 解析前 8 字节 big-endian 时间戳 ---
    guint64 ts_be = 0;
    memcpy(&ts_be, map.data, sizeof(ts_be));

    guint64 send_ts_us =
        ((guint64)map.data[0] << 56) |
        ((guint64)map.data[1] << 48) |
        ((guint64)map.data[2] << 40) |
        ((guint64)map.data[3] << 32) |
        ((guint64)map.data[4] << 24) |
        ((guint64)map.data[5] << 16) |
        ((guint64)map.data[6] << 8)  |
        ((guint64)map.data[7]);

    guint64 now_us = g_get_real_time();
    double latency_ms = (double)(now_us - send_ts_us) / 1000.0;

    g_print("Receiver: Latency = %.2f ms (payload %zu bytes)\n",
            latency_ms, orig_size);

    // --- 构造新的 buffer（去掉前 8 字节） ---
    gsize new_size = orig_size - sizeof(ts_be);
    GstBuffer *newbuf = gst_buffer_new_allocate(NULL, new_size, NULL);

    GstMapInfo newmap;
    gst_buffer_map(newbuf, &newmap, GST_MAP_WRITE);
    memcpy(newmap.data, map.data + sizeof(ts_be), new_size);
    gst_buffer_unmap(newbuf, &newmap);

    gst_buffer_unmap(orig, &map);

    // 保留时间戳信息
    GST_BUFFER_PTS(newbuf) = GST_BUFFER_PTS(orig);
    GST_BUFFER_DTS(newbuf) = GST_BUFFER_DTS(orig);

    // ----------- 关键：旧版 GStreamer 正确做法 -----------
    // 直接替换 probe info 中的 buffer
    GST_PAD_PROBE_INFO_DATA(info) = newbuf;

    // 释放原 buffer
    gst_buffer_unref(orig);

    return GST_PAD_PROBE_OK;
}


int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstElement *pipeline, *udpsrc, *rtpjpegdepay, *jpegparse, *jpegdec, *videoconvert, *sink;
    GstBus *bus;
    GstMessage *msg;

    pipeline = gst_pipeline_new("mjpeg-rtp-pipeline-recv");
    udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    rtpjpegdepay = gst_element_factory_make("rtpjpegdepay", "rtpjpegdepay");
    jpegparse = gst_element_factory_make("jpegparse", "jpegparse");
    jpegdec = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    sink = gst_element_factory_make("xvimagesink", "sink");

    if (!pipeline || !udpsrc || !rtpjpegdepay || !jpegparse || !jpegdec || !videoconvert || !sink) {
        g_printerr("Receiver: not all elements could be created\n");
        return -1;
    }

    g_object_set(udpsrc, "port", 5000, NULL);

    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
                                        "media", G_TYPE_STRING, "video",
                                        "encoding-name", G_TYPE_STRING, "JPEG",
                                        "payload", G_TYPE_INT, 26,
                                        NULL);
    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN(pipeline),
                     udpsrc, rtpjpegdepay, jpegparse, jpegdec, videoconvert, sink,
                     NULL);

    if (!gst_element_link_many(udpsrc, rtpjpegdepay, jpegparse, jpegdec, videoconvert, sink, NULL)) {
        g_printerr("Receiver: failed to link elements\n");
        return -1;
    }

    // 在 depay 的 src pad 插入 probe：depay 输出的 payload 已经是 JPEG 数据（但我们的前缀仍在）
    GstPad *probe_pad = gst_element_get_static_pad(rtpjpegdepay, "src");
    gst_pad_add_probe(probe_pad, GST_PAD_PROBE_TYPE_BUFFER, recv_probe_strip_ts, NULL, NULL);
    gst_object_unref(probe_pad);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (msg) gst_message_unref(msg);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
