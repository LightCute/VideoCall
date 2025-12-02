#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *udpsrc, *rtpjpegdepay, *jpegparse, *jpegdec, *videoconvert, *videosink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    udpsrc       = gst_element_factory_make("udpsrc", "udpsrc");
    rtpjpegdepay = gst_element_factory_make("rtpjpegdepay", "rtpjpegdepay");
    jpegparse    = gst_element_factory_make("jpegparse", "jpegparse");
    jpegdec      = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    videosink    = gst_element_factory_make("xvimagesink", "videosink");

    if (!udpsrc || !rtpjpegdepay || !jpegparse || !jpegdec || !videoconvert || !videosink) {
        g_printerr("Failed to create elements.\n");
        return -1;
    }

    pipeline = gst_pipeline_new("mjpeg-rtp-recv");

    g_object_set(udpsrc, "port", 5000, NULL);

    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
        "media", G_TYPE_STRING, "video",
        "encoding-name", G_TYPE_STRING, "JPEG",
        "payload", G_TYPE_INT, 26,
        NULL);
    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);

    gst_bin_add_many(GST_BIN(pipeline),
        udpsrc, rtpjpegdepay, jpegparse, jpegdec, videoconvert, videosink, NULL);

    if (!gst_element_link_many(udpsrc, rtpjpegdepay, jpegparse, jpegdec, videoconvert, videosink, NULL)) {
        g_printerr("Failed to link elements.\n");
        return -1;
    }

    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set pipeline to PLAYING.\n");
        return -1;
    }

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
         (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg) {
        GError *err;
        gchar *debug;
        gst_message_parse_error(msg, &err, &debug);
        g_printerr("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        gst_message_unref(msg);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
