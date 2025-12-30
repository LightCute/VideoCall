#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    video_ = new VideoWidget(this);
    adapter_ = new QtCameraAdapter(video_, this);
    camera_.setFrameCallback([this](const Frame& f) {
        QMetaObject::invokeMethod(
            adapter_,
            "onFrame",
            Qt::QueuedConnection,
            Q_ARG(Frame, f)
            );
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_Bt_video_on_off_clicked()
{
    qDebug("Button clicked!");
    camera_.start("/dev/video0");
}


void Widget::on_Bt_video_off_clicked()
{
    qDebug("Button clicked!");
    camera_.stop();
}

