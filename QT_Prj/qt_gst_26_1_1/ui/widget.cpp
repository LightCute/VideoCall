//widget.cpp
#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    video_ = new VideoWidget(ui->videoContainer);
    // video_->setSizePolicy(QSizePolicy::Expanding,
    //                       QSizePolicy::Expanding);
    adapter_ = new QtCameraAdapter(video_, this);


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


void Widget::on_Bt_tcp_send_clicked()
{
}


void Widget::on_Bt_tcp_connect_clicked()
{
}

