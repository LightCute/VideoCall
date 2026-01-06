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
    cmdSocket_ = new CommandSocket();

    // 设置回调函数，线程安全调用 Qt UI
    cmdSocket_->setMessageCallback([this](const std::string& msg){
        QString qmsg = QString::fromStdString(msg);
        // 线程安全调用 Qt UI
        QMetaObject::invokeMethod(this, [this, qmsg](){
            ui->Text_tcp_recv_text->append(qmsg); // 假设有 textEdit 显示消息
            //qDebug("%s\n",qmsg);
        });
    });
    // 作为客户端连接服务器
    //cmdSocket_->connectToServer("127.0.0.1", 6000);
    // 或者作为服务器：
    cmdSocket_->startServer(6000);
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


void Widget::on_Bt_tcp_send_clicked()
{
    QString qmsg = ui->lineEdit_tcp_send->text();
    std::string msg = qmsg.toStdString();
    cmdSocket_->sendMessage(msg);
}


void Widget::on_Bt_tcp_connect_clicked()
{
    cmdSocket_->connectToServer("127.0.0.1", 6000);
}

