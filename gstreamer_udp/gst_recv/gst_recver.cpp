#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *udpsrc, *rtpjpegdepay, *jpegdec, *videoconvert, *videosink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    /* Create elements */
    udpsrc        = gst_element_factory_make("udpsrc", "udpsrc");
    rtpjpegdepay  = gst_element_factory_make("rtpjpegdepay", "rtpjpegdepay");
    jpegdec       = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert  = gst_element_factory_make("videoconvert", "videoconvert");
    videosink     = gst_element_factory_make("xvimagesink", "videosink");

    if (!udpsrc || !rtpjpegdepay || !jpegdec || !videoconvert || !videosink) {
        g_printerr("Failed to create some elements.\n");
        return -1;
    }

    /* Create pipeline */
    pipeline = gst_pipeline_new("mjpeg-recv-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create pipeline.\n");
        return -1;
    }

    /* Set udpsrc port */
    g_object_set(udpsrc, "port", 5000, NULL);

    /* Set MJPEG RTP caps */
    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
                                        "media", G_TYPE_STRING, "video",
                                        "encoding-name", G_TYPE_STRING, "JPEG",
                                        "payload", G_TYPE_INT, 26,
                                        NULL);

    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);

    /* Add all elements */
    gst_bin_add_many(GST_BIN(pipeline),
                      udpsrc, rtpjpegdepay, jpegdec, videoconvert, videosink,
                      NULL);

    /* Link them */
    if (!gst_element_link_many(udpsrc, rtpjpegdepay, jpegdec, videoconvert, videosink, NULL)) {
        g_printerr("Failed to link elements!\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Start pipeline */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set pipeline to PLAYING.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Wait for error/EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Handle message */
    if (msg) {
        GError *err;
        gchar *debug_info;
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received: %s\n", err->message);
        g_error_free(err);
        g_free(debug_info);
        gst_message_unref(msg);
    }

    /* Cleanup */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
