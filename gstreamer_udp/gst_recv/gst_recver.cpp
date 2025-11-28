#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *udpsrc, *rtph264depay, *h264parse, *avdec_h264, *videoconvert, *autovideosink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    udpsrc = gst_element_factory_make("udpsrc", "udpsrc");
    rtph264depay = gst_element_factory_make("rtph264depay", "rtph264depay");
    h264parse = gst_element_factory_make("h264parse", "h264parse");
    avdec_h264 = gst_element_factory_make("avdec_h264", "avdec_h264");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    //autovideosink = gst_element_factory_make("autovideosink", "autovideosink");
    autovideosink = gst_element_factory_make("xvimagesink", "xvimagesink");

    /* Check if all elements were created successfully */
    if (!udpsrc || !rtph264depay || !h264parse || !avdec_h264 || !videoconvert || !autovideosink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new("video-receive-pipeline");

    if (!pipeline) {
        g_printerr("Failed to create pipeline.\n");
        return -1;
    }

    /* Set properties */
    g_object_set(udpsrc, "port", 5000, NULL);  // Set the UDP port to receive data from

    /* Create and set the caps for the video format, resolution, and framerate */
    GstCaps *caps = gst_caps_new_simple("application/x-rtp",
                                       "media", G_TYPE_STRING, "video",
                                       "encoding-name", G_TYPE_STRING, "H264",
                                       "payload", G_TYPE_INT, 96,  // The payload type depends on the RTP setup
                                       NULL);
    g_object_set(udpsrc, "caps", caps, NULL);
    gst_caps_unref(caps);  // Unreference since it's now set

    /* Add elements to the pipeline */
    gst_bin_add_many(GST_BIN(pipeline), udpsrc, rtph264depay, h264parse, avdec_h264, videoconvert, autovideosink, NULL);

    /* Link the elements */
    if (gst_element_link_many(udpsrc, rtph264depay, h264parse, avdec_h264, videoconvert, autovideosink, NULL) != TRUE) {
        g_printerr("Elements could not be linked for video processing.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Start playing the pipeline */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Wait until error or EOS (End of Stream) */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Handle messages */
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("GStreamer error: %s\n", err->message);
            g_error_free(err);
            g_free(debug_info);
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
            g_print("End of stream\n");
        }

        gst_message_unref(msg);
    }

    /* Clean up */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
