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

    // 移除：删除 QTimer 轮询逻辑
    // QTimer* timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, [this]{ ... });
    // timer->start(10);
}

LoginWidget::~LoginWidget() {
    // 析构时移除监听者（避免野指针）
    if (core_) {
        core_->removeListener(this);
    }
    delete ui;
}


// 实现 ICoreListener 接口（Core 线程调用，绝不操作 UI）
void LoginWidget::onCoreOutput(const core::CoreOutput& out) {
    // 仅转发为 Qt 信号，交给 UI 线程处理
    emit coreOutputReceived(out);
}

// UI 线程处理 Core 事件（安全操作 UI）
void LoginWidget::handleCoreOutput(const core::CoreOutput& out) {
    std::visit([this](auto&& e) {
        handle(e);
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
    core_->postInput(core::InCmdConnect{"127.0.0.1",6001});
}

void LoginWidget::on_Bt_tcp_test_send_clicked()
{
    QString qmsg = ui->LE_tcp_send_test->text();
    std::string msg = qmsg.toStdString();
}

// 所有 handle 函数参数补充 core:: 前缀
void LoginWidget::handle(const core::OutLoginFail&) {
    std::cout << "[UI] handle OutLoginFail: " << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("Login failed");
}

void LoginWidget::handle(const core::OutDisconnected&) {
    std::cout << "[UI] handle OutDisconnected: " << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("OutDisconnected");
}

void LoginWidget::handle(const core::OutLoginOk&) {
    std::cout << "[UI] handle OutLoginOk: " << std::endl;
    ui->TextEdit_tcp_test_recv->setPlainText("Login success");
    emit loginSuccess();
}

void LoginWidget::handle(const core::OutStateChanged& e) {
    std::cout << "[UI] handle OutStateChanged: " << stateToString(e.from) << " to " << stateToString(e.to) << std::endl;
    ui->TextEdit_FSM_State->setPlainText(QString::fromStdString(stateToString(e.to)));
}

void LoginWidget::handle(const core::OutConnect&) {
    std::cout << "[UI] Ignore OutConnect (CoreExecutor handles it)" << std::endl;
}

void LoginWidget::handle(const core::OutSendLogin&) {
    std::cout << "[UI] Ignore OutSendLogin (CoreExecutor handles it)" << std::endl;
}

void LoginWidget::handle(const core::OutSendPing&) {
    std::cout << "[UI] Ignore OutSendPing (CoreExecutor handles it)" << std::endl;
}

void LoginWidget::handle(const core::OutUpdateAlive&) {
    std::cout << "[UI] Ignore OutUpdateAlive" << std::endl;
}

void LoginWidget::handle(const core::OutOnlineUsers&) {
    std::cout << "[UI] OutOnlineUsers" << std::endl;
}

void LoginWidget::handle(const core::OutSelectLan&) {
    std::cout << "[UI] OutSelectLan" << std::endl;
}

void LoginWidget::handle(const core::OutSelectVpn&) {
    std::cout << "[UI] OutSelectVpn" << std::endl;
}

void LoginWidget::handle(const core::OutSendText& e) {
    std::cout << "[UI] OutSendText  "  << std::endl;
}

// 新增：处理接收转发文本消息（显示到UI）
void LoginWidget::handle(const core::OutForwardText& e) {
    std::cout << "[UI] OutForwardText " << std::endl;

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
