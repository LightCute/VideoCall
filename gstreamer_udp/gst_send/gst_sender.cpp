#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *v4l2src, *videoconvert, *capsfilter, *tee, *queue1, *queue2, *x264enc, *rtph264pay, *udpsink, *autovideosink;
    GstBus *bus;
    GstMessage *msg;
    GstStateChangeReturn ret;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);

    /* Create the elements */
    v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    tee = gst_element_factory_make("tee", "t"); // Give tee element the name "t"
    queue1 = gst_element_factory_make("queue", "queue1");
    queue2 = gst_element_factory_make("queue", "queue2");
    x264enc = gst_element_factory_make("x264enc", "x264enc");
    rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    udpsink = gst_element_factory_make("udpsink", "udpsink");
    //autovideosink = gst_element_factory_make("autovideosink", "autovideosink");
    autovideosink = gst_element_factory_make("xvimagesink", "xvimagesink");

    /* Create the empty pipeline */
    pipeline = gst_pipeline_new("video-capture-pipeline");

    if (!pipeline || !v4l2src || !videoconvert || !capsfilter || !tee || !queue1 || !queue2 || !x264enc || !rtph264pay || !udpsink || !autovideosink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    /* Set properties */
    g_object_set(v4l2src, "device", "/dev/video0", NULL);
    g_object_set(udpsink, "host", "10.0.0.4", "port", 5000, "sync", FALSE, NULL);

    /* Set x264enc parameters for low latency and fast encoding */
    // g_object_set(x264enc, "tune", 0, "speed-preset", 4, NULL);  // tune=zerolatency, speed-preset=ultrafast
    g_object_set(x264enc, "tune", 0, "speed-preset", 4, "key-int-max", 30, "bitrate", 500, NULL);
    



    /* Create and set the caps for the video format, resolution, and framerate */
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                       "format", G_TYPE_STRING, "NV12",
                                       "width", G_TYPE_INT, 640,
                                       "height", G_TYPE_INT, 480,
                                       "framerate", GST_TYPE_FRACTION, 30, 1,
                                       NULL);
    g_object_set(capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);  // Unreference since it's now set

    /* Add all elements to the pipeline */
    gst_bin_add_many(GST_BIN(pipeline), v4l2src, videoconvert, capsfilter, tee, queue1, queue2, x264enc, rtph264pay, udpsink, autovideosink, NULL);

    /* Link the elements for video capture and display */
    if (gst_element_link_many(v4l2src, videoconvert, capsfilter, tee, NULL) != TRUE) {
        g_printerr("Elements could not be linked for video processing.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Link the first queue to the autovideosink for display */
    if (gst_element_link_many(queue1, autovideosink, NULL) != TRUE) {
        g_printerr("Elements could not be linked for display.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Link the second queue to the x264 encoder, RTP payloader, and UDPSink for network transmission */
    if (gst_element_link_many(queue2, x264enc, rtph264pay, udpsink, NULL) != TRUE) {
        g_printerr("Elements could not be linked for network transmission.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Manually link the tee's pads */
    GstPad *tee_audio_pad = gst_element_request_pad_simple(tee, "src_0");
    GstPad *queue_audio_pad = gst_element_get_static_pad(queue1, "sink");
    GstPad *tee_video_pad = gst_element_request_pad_simple(tee, "src_1");
    GstPad *queue_video_pad = gst_element_get_static_pad(queue2, "sink");

    if (gst_pad_link(tee_audio_pad, queue_audio_pad) != GST_PAD_LINK_OK || gst_pad_link(tee_video_pad, queue_video_pad) != GST_PAD_LINK_OK) {
        g_printerr("Tee could not be linked.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Set queue buffer size */
    g_object_set(queue1, "max-size-buffers", 5, NULL);
    g_object_set(queue2, "max-size-buffers", 5, NULL);
    g_object_set(queue1, "leaky", 2, NULL);  // 2 表示丢弃最旧的帧
    g_object_set(queue2, "leaky", 2, NULL);


    /* Start playing the pipeline */
    ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    /* Wait until error or EOS */
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
