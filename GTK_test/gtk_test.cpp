// gst_cam_gtk.cpp
// Compile with:
// gcc gst_cam_gtk.c -o gst_cam_gtk `pkg-config --cflags --libs gstreamer-1.0 gtk+-3.0 gstreamer-video-1.0`

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <glob.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    GtkWidget *window;
    GtkWidget *device_combo;
    GtkWidget *resolution_combo;
    GtkWidget *start_btn;
    GtkWidget *stop_btn;
    GtkWidget *status_label;
    GtkWidget *video_box;       // placeholder for video widget

    GstElement *pipeline;
    GstElement *src;
    GstElement *capsfilter;
    GstElement *videoconvert;
    GstElement *videosink;
} AppData;

/* helper: scan /dev/video* and add to combo */
static void populate_video_devices(GtkComboBoxText *combo) {
    glob_t g;
    int ret = glob("/dev/video*", 0, NULL, &g);
    gtk_combo_box_text_remove_all(combo);
    if (ret == 0) {
        for (size_t i=0;i<g.gl_pathc;i++) {
            // add node (e.g. /dev/video0)
            gtk_combo_box_text_append_text(combo, g.gl_pathv[i]);
        }
    }
    globfree(&g);
}

/* build caps string for resolution selection */
static const char* resolution_to_string(int idx) {
    // index -> resolution string (you can expand)
    switch(idx) {
        case 0: return "width=640,height=480,framerate=30/1";
        case 1: return "width=1280,height=720,framerate=30/1";
        case 2: return "width=1920,height=1080,framerate=30/1";
        case 3: return "width=320,height=240,framerate=30/1";
        default: return "width=640,height=480,framerate=30/1";
    }
}

/* create pipeline elements but do NOT set PLAY yet */
static gboolean create_pipeline(AppData *d) {
    // if exists, destroy first
    if (d->pipeline) {
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        gst_object_unref(d->pipeline);
        d->pipeline = NULL;
    }

    d->src = gst_element_factory_make("v4l2src", "v4lsrc");
    d->capsfilter = gst_element_factory_make("capsfilter", "caps");
    d->videoconvert = gst_element_factory_make("videoconvert", "vconv");
    d->videosink = gst_element_factory_make("gtksink", "sink"); // gtksink provides widget property

    if (!d->src || !d->capsfilter || !d->videoconvert || !d->videosink) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Failed to create GStreamer elements.");
        return FALSE;
    }

    d->pipeline = gst_pipeline_new("camera-pipeline");
    gst_bin_add_many(GST_BIN(d->pipeline), d->src, d->capsfilter, d->videoconvert, d->videosink, NULL);

    // link: src -> capsfilter -> videoconvert -> sink
    if (!gst_element_link(d->src, d->capsfilter)) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Failed to link src -> capsfilter");
        return FALSE;
    }
    if (!gst_element_link(d->capsfilter, d->videoconvert)) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Failed to link capsfilter -> videoconvert");
        return FALSE;
    }
    if (!gst_element_link(d->videoconvert, d->videosink)) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Failed to link videoconvert -> videosink");
        return FALSE;
    }

    return TRUE;
}

/* start pipeline with selected device/resolution */
static void start_pipeline(GtkButton *btn, AppData *d) {
    const gchar *device = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(d->device_combo));
    int res_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(d->resolution_combo));
    if (!device) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "No device selected.");
        return;
    }

    if (!create_pipeline(d)) {
        // error set in create_pipeline
        return;
    }

    // set device property
    g_object_set(d->src, "device", device, NULL);

    // set caps according to resolution
    const char *res_s = resolution_to_string(res_idx < 0 ? 0 : res_idx);
    gchar *caps_str = g_strdup_printf("video/x-raw,%s", res_s);
    GstCaps *caps = gst_caps_from_string(caps_str);
    g_object_set(d->capsfilter, "caps", caps, NULL);
    gst_caps_unref(caps);
    g_free(caps_str);

    // get the gtksink's widget and pack into GUI
    GtkWidget *video_widget = NULL;
    g_object_get(d->videosink, "widget", &video_widget, NULL);
    if (video_widget) {
        // remove previous children of video_box
        GList *kids = gtk_container_get_children(GTK_CONTAINER(d->video_box));
        for (GList *it = kids; it; it = it->next) {
            gtk_container_remove(GTK_CONTAINER(d->video_box), GTK_WIDGET(it->data));
        }
        if (kids) g_list_free(kids);

        gtk_box_pack_start(GTK_BOX(d->video_box), video_widget, TRUE, TRUE, 0);
        gtk_widget_show_all(d->video_box);
    } else {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Warning: gtksink didn't provide widget.");
    }

    // set bus watch for errors and messages
    GstBus *bus = gst_element_get_bus(d->pipeline);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(bus, "message", G_CALLBACK(+[](GstBus *bus, GstMessage *msg, gpointer user_data){
        AppData *ad = (AppData*)user_data;
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err = NULL;
            gchar *dbg = NULL;
            gst_message_parse_error(msg, &err, &dbg);
            gchar *s = g_strdup_printf("Pipeline error: %s", err->message);
            gtk_label_set_text(GTK_LABEL(ad->status_label), s);
            g_free(s);
            g_clear_error(&err);
            g_free(dbg);
            // stop pipeline
            gst_element_set_state(ad->pipeline, GST_STATE_NULL);
        } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_STATE_CHANGED) {
            // ignore internal state messages except from pipeline
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(ad->pipeline)) {
                GstState olds, news, pend;
                gst_message_parse_state_changed(msg, &olds, &news, &pend);
                gchar *s = g_strdup_printf("State changed: %s -> %s",
                                          gst_element_state_get_name(olds),
                                          gst_element_state_get_name(news));
                gtk_label_set_text(GTK_LABEL(ad->status_label), s);
                g_free(s);
            }
        }
    }), d);
    gst_object_unref(bus);

    // set pipeline to playing
    GstStateChangeReturn ret = gst_element_set_state(d->pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        gtk_label_set_text(GTK_LABEL(d->status_label), "Failed to set pipeline to PLAYING");
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        return;
    }

    gtk_widget_set_sensitive(d->start_btn, FALSE);
    gtk_widget_set_sensitive(d->stop_btn, TRUE);
    gtk_label_set_text(GTK_LABEL(d->status_label), "Playing");
}

/* stop pipeline */
static void stop_pipeline(GtkButton *btn, AppData *d) {
    if (d->pipeline) {
        gst_element_set_state(d->pipeline, GST_STATE_NULL);
        gst_object_unref(d->pipeline);
        d->pipeline = NULL;
    }
    // remove video widget children
    GList *kids = gtk_container_get_children(GTK_CONTAINER(d->video_box));
    for (GList *it = kids; it; it = it->next) {
        gtk_container_remove(GTK_CONTAINER(d->video_box), GTK_WIDGET(it->data));
    }
    if (kids) g_list_free(kids);

    gtk_widget_set_sensitive(d->start_btn, TRUE);
    gtk_widget_set_sensitive(d->stop_btn, FALSE);
    gtk_label_set_text(GTK_LABEL(d->status_label), "Stopped");
}

/* Refresh device list (button) */
static void on_refresh(GtkButton *btn, AppData *d) {
    populate_video_devices(GTK_COMBO_BOX_TEXT(d->device_combo));
}

/* Build UI and connect handlers */
static void build_ui(AppData *d) {
    d->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(d->window), "Camera Viewer (GStreamer + GTK)");
    gtk_window_set_default_size(GTK_WINDOW(d->window), 900, 500);
    g_signal_connect(d->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 6);
    gtk_container_add(GTK_CONTAINER(d->window), main_box);

    // left: video area (box to hold gtksink widget dynamically)
    d->video_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), d->video_box, TRUE, TRUE, 0);

    // right: controls
    GtkWidget *ctrl_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_pack_start(GTK_BOX(main_box), ctrl_box, FALSE, FALSE, 0);

    // device selection
    gtk_box_pack_start(GTK_BOX(ctrl_box), gtk_label_new("Camera device:"), FALSE, FALSE, 0);
    d->device_combo = gtk_combo_box_text_new();
    populate_video_devices(GTK_COMBO_BOX_TEXT(d->device_combo));
    gtk_box_pack_start(GTK_BOX(ctrl_box), d->device_combo, FALSE, FALSE, 0);

    GtkWidget *refresh_btn = gtk_button_new_with_label("Refresh devices");
    g_signal_connect(refresh_btn, "clicked", G_CALLBACK(on_refresh), d);
    gtk_box_pack_start(GTK_BOX(ctrl_box), refresh_btn, FALSE, FALSE, 0);

    // resolution
    gtk_box_pack_start(GTK_BOX(ctrl_box), gtk_label_new("Resolution / FPS:"), FALSE, FALSE, 0);
    d->resolution_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(d->resolution_combo), "640x480 @30");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(d->resolution_combo), "1280x720 @30");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(d->resolution_combo), "1920x1080 @30");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(d->resolution_combo), "320x240 @30");
    gtk_combo_box_set_active(GTK_COMBO_BOX(d->resolution_combo), 0);
    gtk_box_pack_start(GTK_BOX(ctrl_box), d->resolution_combo, FALSE, FALSE, 0);

    // start / stop
    d->start_btn = gtk_button_new_with_label("Start");
    d->stop_btn  = gtk_button_new_with_label("Stop");
    gtk_widget_set_sensitive(d->stop_btn, FALSE);
    g_signal_connect(d->start_btn, "clicked", G_CALLBACK(start_pipeline), d);
    g_signal_connect(d->stop_btn,  "clicked", G_CALLBACK(stop_pipeline), d);

    gtk_box_pack_start(GTK_BOX(ctrl_box), d->start_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ctrl_box), d->stop_btn,  FALSE, FALSE, 0);

    // status
    gtk_box_pack_start(GTK_BOX(ctrl_box), gtk_label_new("Status:"), FALSE, FALSE, 0);
    d->status_label = gtk_label_new("Idle");
    gtk_box_pack_start(GTK_BOX(ctrl_box), d->status_label, FALSE, FALSE, 0);

    gtk_widget_show_all(d->window);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);
    gtk_init(&argc, &argv);

    AppData data;
    memset(&data, 0, sizeof(data));
    data.pipeline = NULL;

    build_ui(&data);

    gtk_main();

    // cleanup if exit while pipeline running
    if (data.pipeline) {
        gst_element_set_state(data.pipeline, GST_STATE_NULL);
        gst_object_unref(data.pipeline);
        data.pipeline = NULL;
    }

    return 0;
}
