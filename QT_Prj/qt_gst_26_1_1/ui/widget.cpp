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

    // 远端视频初始化（和本地完全对称）
    remote_video_ = new VideoWidget(ui->remoteVideoContainer); // 需在UI设计师中添加remoteVideoContainer控件
    remote_adapter_ = new QtCameraAdapter(remote_video_, this);
    receiver_.setFrameCallback([this](const Frame& f) {
        // 跨线程调用（接收端在GStreamer线程，UI在主线程）
        QMetaObject::invokeMethod(remote_adapter_, "onFrame", Qt::QueuedConnection, Q_ARG(Frame, f));
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
    // 停止接收端
    receiver_.stop();
    // 停止发送端（原有逻辑）
    camera_.stop();
    delete ui;
}

// 新接口实现：只转发信号
void Widget::onUiOutput(const core::UiOutput& out)
{
    qDebug() << "Main UI received core ui output, type index:" << out.index();
    emit coreOutputReceived(out);
}
void Widget::handleCoreOutput(const core::UiOutput& out)
{
    std::visit([this, &out](auto&& e) {
        using T = std::decay_t<decltype(e)>; // 获取移除引用/const 后的原始类型
        // 匹配 Widget 关心的所有类型
        if constexpr (std::is_same_v<T, core::UiOutStateChanged>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutLoginOk>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutLoginFail>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutDisconnected>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutOnlineUsers>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutForwardText>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutShowIncomingCall>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutMediaReadyFinal>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutCallEnded>) {
            handle(e);
        } else if constexpr (std::is_same_v<T, core::UiOutStopMedia>) {
            handle(e);
        }
        // 兜底：未处理的 UiOutput 类型，打印日志（避免潜在问题，方便后续调试）
        else {
            std::cerr << "[Widget] Unhandled UiOutput type, index: " << out.index() << std::endl;
        }
    }, out);
}

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


void Widget::on_Bt_set_lan_clicked()
{
    core_->postInput(core::InSelectLan{}); // 触发LAN模式
}


void Widget::on_Bt_sen_vpn_clicked()
{
    core_->postInput(core::InSelectVpn{}); // 触发VPN模式
}


void Widget::on_Bt_Hangup_clicked()
{
    core_->postInput(core::InCmdHangup{});
}


// 只保留 UiOutXXX 相关 handle
void Widget::handle(const core::UiOutStateChanged& e) {
    std::cout << "[Widget] FSM:"
              << stateToString(e.from)
              << "→"
              << stateToString(e.to) << std::endl;
    ui -> PTE_state ->setPlainText(QString::fromStdString(stateToString(e.to)));
}

void Widget::handle(const core::UiOutLoginOk&) {
    std::cout << "[Widget] UiOutLoginOk" << std::endl;
}

void Widget::handle(const core::UiOutLoginFail& e) {
    std::cout << "[Widget] UiOutLoginFail: " << e.msg << std::endl;
    QMessageBox::warning(this, "Login Failed", QString::fromStdString(e.msg));
}

void Widget::handle(const core::UiOutDisconnected&) {
    std::cout << "[Widget] UiOutDisconnected" << std::endl;
    QMessageBox::information(this, "Info", "Disconnected from server");
}

void Widget::handle(const core::UiOutOnlineUsers& e) {
    std::cout << "[UI] UiOutOnlineUsers" << std::endl;
    QString text;
    for (const auto& u : e.list) {
        text += QString("%1 (priv=%2)\n")
                    .arg(QString::fromStdString(u.name))
                    .arg(u.privilege);
    }
    ui->text_onlineUsers->setPlainText(text);
}

void Widget::handle(const core::UiOutForwardText& e) {
    std::cout << "[Widget] recev from " << e.from_user << " msg: " << e.content << std::endl;
    QString text = ui->PTE_recv->toPlainText();
    text += QString("[%1]: %2\n").arg(QString::fromStdString(e.from_user)).arg(QString::fromStdString(e.content));
    ui->PTE_recv->setPlainText(text);
}

void Widget::handle(const core::UiOutShowIncomingCall& e) {
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

void Widget::handle(const core::UiOutMediaReadyFinal& e) {
    std::cout << "[Widget] UiOutMediaReadyFinal" << std::endl;
    std::cout << "[UI] Media ready, peer IP: " << e.peerIp << ", port: " << e.peerPort << std::endl;

    int mediaPort = core_->getMediaPort();
    if (mediaPort <= 0) {
        std::cerr << "[Widget] Invalid media port: " << mediaPort << std::endl;
        QMessageBox::warning(this, "Error", "No available UDP port found!");
        return;
    }

    camera_.start("/dev/video0", e.peerIp, e.peerPort);
    receiver_.start(mediaPort);
}

void Widget::handle(const core::UiOutCallEnded& e) {
    std::cout << "[Widget] Call ended: peer=" << e.peer << ", reason=" << e.reason << std::endl;
    ui->lineEdit_CallTarget->clear();
}

void Widget::handle(const core::UiOutStopMedia&) {
    std::cout << "[Widget] Stop media" << std::endl;
    camera_.stop();
    receiver_.stop();
    video_->setFrame(QImage());
    remote_video_->setFrame(QImage());
}
