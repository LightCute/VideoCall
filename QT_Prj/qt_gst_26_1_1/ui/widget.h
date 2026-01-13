//widget.h
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QtCameraAdapter.h"
#include "CameraManager.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_Bt_video_on_off_clicked();

    void on_Bt_video_off_clicked();

    void on_Bt_tcp_send_clicked();

    void on_Bt_tcp_connect_clicked();

private:
    Ui::Widget *ui;
    VideoWidget *video_;
    CameraManager camera_;
    QtCameraAdapter* adapter_;
};
#endif // WIDGET_H
