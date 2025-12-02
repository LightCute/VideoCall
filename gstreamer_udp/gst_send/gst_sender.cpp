#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *v4l2src, *capsfilter, *tee, *queue1, *queue2;
    GstElement *jpegdec, *videoconvert, *rtpjpegpay, *udpsink, *autovideosink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    gst_init(&argc, &argv);

    /* Create elements */
    v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    tee = gst_element_factory_make("tee", "tee");
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
    jpegdec = gst_element_factory_make("jpegdec", "jpegdec");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    rtpjpegpay = gst_element_factory_make("rtpjpegpay", "rtpjpegpay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");
    autovideosink = gst_element_factory_make("xvimagesink", "xvimagesink");

    if (!v4l2src || !capsfilter || !tee || !queue1 || !queue2 || !jpegdec ||
        !videoconvert || !rtpjpegpay || !udpsink || !autovideosink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Create the pipeline */
    pipeline = gst_pipeline_new("mjpeg-rtp-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create pipeline.\n");
        return -1;
    }

    /* Configure elements */
    g_object_set(v4l2src, "device", "/dev/video4", NULL);

    /* udpsink: set destination IP and port (adjust as needed) */
    g_object_set(udpsink,
                 "host", "10.0.0.4",
                 "port", 5000,
                 "sync", FALSE,
                 "async", FALSE,
                 NULL);

    /* Keep queues tiny and leaky (drop old frames) for low latency */
    g_object_set(queue1, "max-size-buffers", 1, "leaky", 2, NULL);
    g_object_set(queue2, "max-size-buffers", 1, "leaky", 2, NULL);

    /* Video display: don't sync to clock to reduce latency */
    g_object_set(autovideosink, "sync", FALSE, NULL);

    /* caps: request MJPEG at 640x480@30 */
    GstCaps *caps = gst_caps_new_simple(
        "image/jpeg",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);

    /* Add elements to pipeline */
    gst_bin_add_many(GST_BIN(pipeline),
                     v4l2src, capsfilter, tee,
                     queue1, jpegdec, videoconvert, autovideosink,
                     queue2, rtpjpegpay, udpsink,
                     NULL);

    /* Link: v4l2src -> capsfilter -> tee */
    if (!gst_element_link_many(v4l2src, capsfilter, tee, NULL)) {
        g_printerr("Failed to link source -> caps -> tee.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Request tee src pads and link to queues */
    GstPad *tee_src_pad1 = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *tee_src_pad2 = gst_element_request_pad_simple(tee, "src_%u");
    GstPad *queue1_sink = gst_element_get_static_pad(queue1, "sink");
    GstPad *queue2_sink = gst_element_get_static_pad(queue2, "sink");

    if (gst_pad_link(tee_src_pad1, queue1_sink) != GST_PAD_LINK_OK ||
        gst_pad_link(tee_src_pad2, queue2_sink) != GST_PAD_LINK_OK) {
        g_printerr("Tee could not be linked to queues.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    gst_object_unref(queue1_sink);
    gst_object_unref(queue2_sink);

    /* Link display branch: queue1 -> jpegdec -> videoconvert -> sink */
    if (!gst_element_link_many(queue1, jpegdec, videoconvert, autovideosink, NULL)) {
        g_printerr("Failed to link display branch.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Link network branch: queue2 -> rtpjpegpay -> udpsink */
    if (!gst_element_link_many(queue2, rtpjpegpay, udpsink, NULL)) {
        g_printerr("Failed to link network branch.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Optional: tune rtpjpegpay (mtu) to control packet size */
    g_object_set(rtpjpegpay, "mtu", 1400, NULL);

    /* Start playing */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                     static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg != NULL) {
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

    /* Cleanup */
    gst_element_set_state(pipeline, GST_STATE_NULL);

    /* release requested tee pads */
    gst_element_release_request_pad(tee, tee_src_pad1);
    gst_element_release_request_pad(tee, tee_src_pad2);
    gst_object_unref(tee_src_pad1);
    gst_object_unref(tee_src_pad2);

    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
