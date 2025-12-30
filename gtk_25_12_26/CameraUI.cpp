#include "CameraUI.h"
#include <filesystem>

CameraUI::CameraUI(CameraWidget* camera)
    : camera_(camera),
      window_(nullptr),
      main_box_(nullptr),
      control_box_(nullptr),
      device_combo_(nullptr),
      start_btn_(nullptr),
      pause_btn_(nullptr) {}

void CameraUI::populate_devices() {
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        auto name = entry.path().filename().string();
        if (name.find("video") == 0) {
            gtk_combo_box_text_append_text(
                GTK_COMBO_BOX_TEXT(device_combo_),
                ("/dev/" + name).c_str()
            );
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(device_combo_), 0);
}

void CameraUI::build() {
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_), "Camera Viewer");
    gtk_window_set_default_size(GTK_WINDOW(window_), 800, 600);
    g_signal_connect(window_, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    main_box_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window_), main_box_);

    /* 控制栏 */
    control_box_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_box_), control_box_, FALSE, FALSE, 0);

    device_combo_ = gtk_combo_box_text_new();
    populate_devices();
    gtk_box_pack_start(GTK_BOX(control_box_), device_combo_, FALSE, FALSE, 0);

    start_btn_ = gtk_button_new_with_label("Start");
    pause_btn_ = gtk_button_new_with_label("Pause");

    gtk_box_pack_start(GTK_BOX(control_box_), start_btn_, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_box_), pause_btn_, FALSE, FALSE, 0);

    g_signal_connect(start_btn_, "clicked",
                     G_CALLBACK(on_start_clicked), this);
    g_signal_connect(pause_btn_, "clicked",
                     G_CALLBACK(on_pause_clicked), this);

    /* 视频区域 */
    GtkWidget* video = camera_->get_widget();
    if (video && !gtk_widget_get_parent(video)) {
        gtk_box_pack_start(GTK_BOX(main_box_), video, TRUE, TRUE, 0);
    }
}

void CameraUI::show() {
    gtk_widget_show_all(window_);
}

/* ====== 回调 ====== */

void CameraUI::on_start_clicked(GtkButton*, gpointer user_data) {
    auto* ui = static_cast<CameraUI*>(user_data);

    gchar* dev = gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(ui->device_combo_)
    );

    if (!dev) return;

    ui->camera_->stop();
    ui->camera_->set_device(dev);
    //ui->camera_->init();
    ui->camera_->start();

    g_free(dev);
}

void CameraUI::on_pause_clicked(GtkButton*, gpointer user_data) {
    auto* ui = static_cast<CameraUI*>(user_data);
    ui->camera_->stop();
}
