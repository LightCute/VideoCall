//LoginWidget.cpp
#include "LoginWidget.h"
#include "./ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>
#include "ClientCore.h"



LoginWidget::LoginWidget(ClientCore* core, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , core_(core)  // 接收 AppWindow 传入的 Core 实例
{
    ui->setupUi(this);

    // 注册为 Core 的监听者
    core_->addListener(this);

    // 关键：跨线程信号槽（强制 QueuedConnection，确保 UI 线程执行）
    connect(this, &LoginWidget::coreOutputReceived,
            this, &LoginWidget::handleCoreOutput,
            Qt::QueuedConnection);


}

LoginWidget::~LoginWidget() {
    // 析构时移除监听者（避免野指针）
    if (core_) {
        core_->removeListener(this);
    }
    delete ui;
}


// 新接口实现
void LoginWidget::onUiOutput(const core::UiOutput& out) {
    emit coreOutputReceived(out);
}

// UI 线程处理 Core 事件（安全操作 UI）- 已添加兜底处理
void LoginWidget::handleCoreOutput(const core::UiOutput& out) {
    std::visit([this, &out](auto&& e) {
        using T = std::decay_t<decltype(e)>; // 获取移除引用/const 后的原始类型
        // 匹配 LoginWidget 关心的所有类型
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
        }
        // 兜底：未处理的 UiOutput 类型，打印日志（避免潜在问题，方便后续调试）
        else {
            std::cerr << "[LoginWidget] Unhandled UiOutput type, index: " << out.index() << std::endl;
        }
    }, out);
}

void LoginWidget::on_Bt_Jump_Test_clicked()
{
    emit loginSuccess();
}

void LoginWidget::on_Bt_ConnectToServer_clicked()
{
    //qDebug() << "current thread:" << QThread::currentThread();
    //qDebug() << "ui thread:" << qApp->thread();

    ui->TextEdit_tcp_test_recv->setPlainText("Connecting\n");
    // 替换为 InCmdConnect
    core_->postInput(core::InCmdConnect{"192.168.6.64",6001});
}

void LoginWidget::on_Bt_tcp_test_send_clicked()
{
    QString qmsg = ui->LE_tcp_send_test->text();
    std::string msg = qmsg.toStdString();
}



void LoginWidget::on_Bt_Login_clicked()
{
    QString qstr_user_name = ui->LE_UserName->text();
    std::string msg_user_name = qstr_user_name.toStdString();

    QString qstr_user_pass = ui->LE_UserPass->text();
    std::string msg_user_pass = qstr_user_pass.toStdString();

    // 替换为 InCmdLogin
    core_->postInput(core::InCmdLogin{msg_user_name, msg_user_pass});
}

// 只保留登录界面关心的事件
void LoginWidget::handle(const core::UiOutStateChanged& e) {
    std::cout << "[UI] State: " << stateToString(e.from) << " → " << stateToString(e.to) << std::endl;
    ui->TextEdit_FSM_State->setPlainText(QString::fromStdString(stateToString(e.to)));
}

void LoginWidget::handle(const core::UiOutLoginOk&) {
    std::cout << "[UI] Login success" << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("Login success");
    emit loginSuccess();
}

void LoginWidget::handle(const core::UiOutLoginFail& e) {
    std::cout << "[UI] Login failed: " << e.msg << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("Login failed: " + QString::fromStdString(e.msg));
}

void LoginWidget::handle(const core::UiOutDisconnected&) {
    std::cout << "[UI] Disconnected" << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("Disconnected");
}

void LoginWidget::handle(const core::UiOutOnlineUsers&) {
    // 登录界面不需要处理在线用户，空实现即可
}

void LoginWidget::handle(const core::UiOutForwardText&) {
    // 登录界面不需要处理消息
}

