#ifndef GTK_GUI_H
#define GTK_GUI_H

#include <gtk/gtk.h>
#include <string>
#include "gst_video.h"

class GtkGui {
public:
    GtkGui();
    ~GtkGui();

    void show();

private:
    GtkWidget *window;
    GtkWidget *combo;
    GtkWidget *startBtn;
    GtkWidget *pauseBtn;

    GtkWidget *vbox;

    GstVideo video;

    void populateCameras();
    static void onStartClicked(GtkButton *, gpointer userData);
    static void onPauseClicked(GtkButton *, gpointer userData);
};

#endif
