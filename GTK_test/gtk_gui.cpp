#include "gtk_gui.h"
#include <iostream>
#include <filesystem>
#include <gst/gst.h>
#include <gst/video/video.h>



GtkGui::GtkGui() {
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    combo = gtk_combo_box_text_new();
    startBtn = gtk_button_new_with_label("Start");
    pauseBtn = gtk_button_new_with_label("Pause");

    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), startBtn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), pauseBtn, FALSE, FALSE, 5);

    populateCameras();


    g_signal_connect(startBtn, "clicked", G_CALLBACK(onStartClicked), this);
    g_signal_connect(pauseBtn, "clicked", G_CALLBACK(onPauseClicked), this);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

GtkGui::~GtkGui() {}

void GtkGui::show() {
    gtk_widget_show_all(window);
}

void GtkGui::populateCameras() {
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));

    for (const auto &entry : std::filesystem::directory_iterator("/dev")) {
        std::string name = entry.path().string();
        if (name.find("video") != std::string::npos) {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), name.c_str());
        }
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
}

void GtkGui::onStartClicked(GtkButton *, gpointer data) {
    GtkGui *gui = (GtkGui *)data;

    gchar *device = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(gui->combo));
    if (!device) return;
    // 创建 gtksink
    GstElement *gtksink;
    gtksink = gst_element_factory_make("gtksink", "video_sink");
    if (!gtksink) {
        std::cerr << "无法创建 gtksink" << std::endl;
        //return -1;
    }

    // 获取 gtksink 的 widget 并加入 GTK 窗口
    GtkWidget *video_area = nullptr;
    g_object_get(gtksink, "widget", &video_area, NULL);
    gtk_widget_set_hexpand(video_area, TRUE);
    gtk_widget_set_vexpand(video_area, TRUE);
    gtk_box_pack_start(GTK_BOX(gui->vbox), video_area, TRUE, TRUE, 0);

    gui->video.build(gtksink);




    g_free(device);
}

void GtkGui::onPauseClicked(GtkButton *, gpointer data) {
    GtkGui *gui = (GtkGui *)data;

    static bool paused = false;

    if (!paused) {
        gui->video.pause();
        gtk_button_set_label(GTK_BUTTON(gui->pauseBtn), "Resume");
    } else {
        gui->video.resume();
        gtk_button_set_label(GTK_BUTTON(gui->pauseBtn), "Pause");
    }

    paused = !paused;
}
