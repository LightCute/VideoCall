#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

int main(int argc, char *argv[])
{
    // 初始化 GTK 与 GStreamer
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    // 创建 GTK 窗口
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Camera Preview");
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 创建 gtksink（用来在 GTK 中显示视频）
    GstElement *sink = gst_element_factory_make("gtksink", NULL);
    if (!sink) {
        g_printerr("无法创建 gtksink，请安装 gstreamer1.0-gtk3\n");
        return -1;
    }

    // gtksink 自带的 GtkWidget，可嵌入到窗口
    GtkWidget *video_widget = (GtkWidget *)g_object_get_data(G_OBJECT(sink), "widget");

    // 将视频 widget 加到窗口
    gtk_container_add(GTK_CONTAINER(window), video_widget);

    // GStreamer 管道（摄像头 → videoconvert → gtksink）
    GstElement *pipeline = gst_parse_launch(
        "v4l2src device=/dev/video0 ! "
        "video/x-raw,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! gtksink name=sink",
        NULL);

    if (!pipeline) {
        g_printerr("无法创建 GStreamer 管道\n");
        return -1;
    }

    // 设定 gtksink 的 widget
    GstElement *pipeline_sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    g_object_set(pipeline_sink, "widget", video_widget, NULL);
    gst_object_unref(pipeline_sink);

    // 播放
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // 显示 GTK 窗口
    gtk_widget_show_all(window);

    // GTK 主循环
    gtk_main();

    // 退出后清理 GStreamer
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}
