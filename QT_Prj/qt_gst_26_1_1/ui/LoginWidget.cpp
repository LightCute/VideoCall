//LoginWidget.cpp
#include "LoginWidget.h"
#include "./ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>

#include <QThread>
#include <QTimer>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    core_ = new ClientCore;

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]{
        core::CoreOutput out; // 补充 core:: 前缀
        while (core_->pollOutput(out)) {
            std::cout << "[UI] get output, type index: " << out.index() << std::endl;
            std::visit([this](const auto& e){
                handle(e);
            }, out);
        }
    });
    timer->start(10);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_Bt_Jump_Test_clicked()
{
    emit loginSuccess();
}

void LoginWidget::on_Bt_ConnectToServer_clicked()
{
    qDebug() << "current thread:" << QThread::currentThread();
    qDebug() << "ui thread:" << qApp->thread();

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

void LoginWidget::on_Bt_Login_clicked()
{
    QString qstr_user_name = ui->LE_UserName->text();
    std::string msg_user_name = qstr_user_name.toStdString();

    QString qstr_user_pass = ui->LE_UserPass->text();
    std::string msg_user_pass = qstr_user_pass.toStdString();

    // 替换为 InCmdLogin
    core_->postInput(core::InCmdLogin{msg_user_name, msg_user_pass});
}
