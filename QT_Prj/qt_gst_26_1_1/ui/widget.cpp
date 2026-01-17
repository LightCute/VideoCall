// ui/widget.cpp（完整修复版）
#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>
#include "ClientCore.h" // 包含Core头文件
#include "ClientState.h" // 包含状态转换函数

Widget::Widget(ClientCore* core, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , core_(core)  // 初始化Core指针
{
    ui->setupUi(this);
    video_ = new VideoWidget(ui->videoContainer);
    adapter_ = new QtCameraAdapter(video_, this);

    // 关键修复：绑定摄像头回调到Qt界面
    camera_.setFrameCallback([this](const Frame& f) {
        // 跨线程安全调用（CameraManager在子线程，UI在主线程）
        QMetaObject::invokeMethod(adapter_, "onFrame", Qt::QueuedConnection, Q_ARG(Frame, f));
    });

    // 关键修复：绑定Core事件的跨线程处理
    connect(this, &Widget::coreOutputReceived,
            this, &Widget::handleCoreOutput,
            Qt::QueuedConnection);
}

Widget::~Widget()
{
    // 修复：使用正确的移除监听者方法
    if (core_) {
        core_->removeListener(this);
    }
    delete ui;
}

// 实现ICoreListener接口：接收Core输出事件
void Widget::onCoreOutput(const core::CoreOutput& out)
{
    qDebug() << "Main UI received core output, type index:" << out.index();
    // 转发为Qt信号（确保UI线程处理）
    emit coreOutputReceived(out);
}

// 实现槽函数：处理Core输出事件（UI线程）
void Widget::handleCoreOutput(const core::CoreOutput& out)
{
    // 可根据需要扩展处理逻辑，目前打印日志
    std::visit([](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, core::OutStateChanged>) {
            qDebug() << "Main UI state change:" << QString::fromStdString(stateToString(e.from))
                     << "→" << QString::fromStdString(stateToString(e.to));
        }
    }, out);
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

void Widget::on_Bt_tcp_send_clicked() {}
void Widget::on_Bt_tcp_connect_clicked() {}
