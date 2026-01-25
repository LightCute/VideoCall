// ui/widget.cpp（完整修复版）
#include "widget.h"
#include "./ui_widget.h"
#include <QDebug>
#include "ClientCore.h" // 包含Core头文件
#include "ClientState.h" // 包含状态转换函数
#include <QMessageBox>
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

    if (core_) {
        core_->addListener(this);
    }

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

void Widget::handleCoreOutput(const core::CoreOutput& out)
{
    std::visit([this](auto&& e) {
        handle(e);   // 🔥 和 LoginWidget 一模一样的分发风格
    }, out);
}

void Widget::handle(const core::OutOnlineUsers& e) {
    std::cout << "[UI] OutOnlineUsers" << std::endl;
    QString text;

    for (const auto& u : e.list) {
        text += QString("%1 (priv=%2)\n")
                    .arg(QString::fromStdString(u.name))
                    .arg(u.privilege);
    }

    ui->text_onlineUsers->setPlainText(text);
}
// // 实现槽函数：处理Core输出事件（UI线程）
// void Widget::handleCoreOutput(const core::CoreOutput& out)
// {
//     // 可根据需要扩展处理逻辑，目前打印日志
//     std::visit([](auto&& e) {
//         using T = std::decay_t<decltype(e)>;
//         if constexpr (std::is_same_v<T, core::OutStateChanged>) {
//             qDebug() << "Main UI state change:" << QString::fromStdString(stateToString(e.from))
//                      << "→" << QString::fromStdString(stateToString(e.to));
//         }
//     }, out);
// }

void Widget::on_Bt_video_on_off_clicked()
{
    qDebug("Button clicked!");
    //camera_.start("/dev/video0");
}

void Widget::on_Bt_video_off_clicked()
{
    qDebug("Button clicked!");
    camera_.stop();
}

void Widget::on_Bt_tcp_send_clicked() {
    // 获取目标用户名和消息内容
    QString target_user = ui->lineEdit_target_user->text();
    QString content = ui->lineEdit_msg->text();
    if (target_user.isEmpty() || content.isEmpty()) {
        std::cout << "can not be empty" << std::endl;
        return;
    }
    // 发送到Core层
    core_->postInput(core::InCmdSendText{
        target_user.toStdString(),
        content.toStdString()
    });
    // 清空输入框
    ui->lineEdit_msg->clear();
}

// 新增：处理发送文本消息（仅日志）
void Widget::handle(const core::OutSendText& e) {
    std::cout << "[Widget] Send text to  " << e.target_user << ": " << e.content << std::endl;
}

// 新增：处理接收转发文本消息（显示到UI）
void Widget::handle(const core::OutForwardText& e) {
    std::cout << "[Widget] recev from " << e.from_user << " msg: " << e.content << std::endl;
    // 追加到文本框
    QString text = ui->PTE_recv->toPlainText();
    text += QString("[%1]: %2\n").arg(QString::fromStdString(e.from_user)).arg(QString::fromStdString(e.content));
    ui->PTE_recv->setPlainText(text);
}

void Widget::handle(const core::OutStateChanged& e) {
    std::cout << "[Widget] FSM:"
             << (stateToString(e.from))
             << "→"
             << (stateToString(e.to)) << std::endl;


}

void Widget::handle(const core::OutDisconnected&) {
    std::cout << "[Widget] OutDisconnected" << std::endl;
}

void Widget::handle(const core::OutConnect&) {
    std::cout << "[Widget] Ignore OutConnect (CoreExecutor handles it)" << std::endl;
}

void Widget::handle(const core::OutSendLogin&) {
    std::cout << "[Widget] Ignore OutSendLogin" << std::endl;
}

void Widget::handle(const core::OutSendPing&) {
    std::cout << "[Widget] Ignore OutSendPing" << std::endl;
}

void Widget::handle(const core::OutUpdateAlive&) {
    std::cout << "[Widget] OutUpdateAlive" << std::endl;
}

void Widget::handle(const core::OutLoginOk&) {
    std::cout << "[Widget] OutLoginOk (optional handling)" << std::endl;
}

void Widget::handle(const core::OutLoginFail&) {
    std::cout << "[Widget] OutLoginFail (optional handling)" << std::endl;
}

void Widget::handle(const core::OutSelectLan&) {
    std::cout << "[Widget] OutSelectLan" << std::endl;
}

void Widget::handle(const core::OutSelectVpn&) {
    std::cout << "[Widget] OutSelectVpn)" << std::endl;
}


//**********************
void Widget::handle(const core::OutSendCall&) {
    std::cout << "[Widget] OutSendCall)" << std::endl;
}

void Widget::handle(const core::OutSendAcceptCall&) {
    std::cout << "[Widget] OutSendAcceptCall)" << std::endl;
}

void Widget::handle(const core::OutSendRejectCall&) {
    std::cout << "[Widget] OutSendRejectCall)" << std::endl;
}

void Widget::handle(const core::OutSendMediaOffer&) {
    std::cout << "[Widget] OutSendMediaOffer)" << std::endl;
}

void Widget::handle(const core::OutSendMediaAnswer&) {
    std::cout << "[Widget] OutSendMediaAnswer)" << std::endl;
}

void Widget::handle(const core::OutMediaReady& e) {
    std::cout << "[Widget] OutMediaReady)" << std::endl;
    std::cout << "[UI] Media ready, peer IP: " << e.peerIp << ", port: " << e.peerPort << std::endl;
    camera_.start("/dev/video0", e.peerIp, e.peerPort);
}

void Widget::handle(const core::OutShowIncomingCall& e) {
    // 弹出来电对话框
    QMessageBox msgBox;
    msgBox.setWindowTitle("Incoming");
    msgBox.setText(QString("User %1 is calling you").arg(QString::fromStdString(e.from)));
    msgBox.addButton("Accept", QMessageBox::AcceptRole);
    msgBox.addButton("Reject", QMessageBox::RejectRole);
    int ret = msgBox.exec();
    if (ret == QMessageBox::AcceptRole) {
        core_->postInput(core::InCmdAcceptCall{});
    } else {
        core_->postInput(core::InCmdRejectCall{});
    }
}



void Widget::on_Bt_Call_clicked()
{
    QString targetUser = ui->lineEdit_CallTarget->text();
    core_->postInput(core::InCmdCall{targetUser.toStdString()});
}


void Widget::on_Bt_AcceptCall_clicked()
{
    core_->postInput(core::InCmdAcceptCall{});
}


void Widget::on_Bt_RejectCall_clicked()
{
    core_->postInput(core::InCmdRejectCall{});
}

