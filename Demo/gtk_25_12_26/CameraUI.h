#pragma once
#include <gtk/gtk.h>
#include "CameraWidget.h"

class CameraUI {
public:
    CameraUI(CameraWidget* camera);
    void build();
    void show();

private:
    CameraWidget* camera_;

    GtkWidget* window_;
    GtkWidget* main_box_;
    GtkWidget* control_box_;
    GtkWidget* device_combo_;
    GtkWidget* start_btn_;
    GtkWidget* pause_btn_;

    void populate_devices();

    static void on_start_clicked(GtkButton*, gpointer);
    static void on_pause_clicked(GtkButton*, gpointer);
};
