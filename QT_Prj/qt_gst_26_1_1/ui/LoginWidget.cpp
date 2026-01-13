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

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        ClientEvent ev;
        while(core_->pollEvent(ev)) {
            std::visit([this](auto&& e){ handle(e); }, ev);
        }
    });
    timer->start(10); // 每 10ms 拉取一次事件


    // core_->onEvent = [this](ClientEvent ev){
    //     auto evCopy = std::move(ev); // 移动一次
    //     QTimer::singleShot(0, this, [this, ev=std::move(evCopy)]() mutable {
    //         std::visit([this](auto&& e){
    //             handle(e);
    //         }, ev);
    //     });
    // };




    core_->onStateChanged = [this](State s){
        QString stateStr;
        switch(s){
        case State::Disconnected: stateStr = "Disconnected"; break;
        case State::Connecting:   stateStr = "Connecting"; break;
        case State::Connected:    stateStr = "Connected"; break;
        case State::LoggingIn:    stateStr = "LoggingIn"; break;
        case State::LoggedIn:     stateStr = "LoggedIn"; break;
        }

        // 显示在 UI 上
        ui->TextEdit_FSM_State->setPlainText("State: " + stateStr);
    };

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

    // if(core_->doconnect("127.0.0.1", 6001) == true)
    // {
    //     ui->TextEdit_tcp_test_recv->setPlainText("Connect sucess\n");
    // }
    // else
    // {
    //     ui->TextEdit_tcp_test_recv->setPlainText("Connect fail\n");
    // }
    ui->TextEdit_tcp_test_recv->setPlainText("Connecting\n");
    core_->postEvent(EvCmdConnect{"127.0.0.1", 6001});


}


void LoginWidget::on_Bt_tcp_test_send_clicked()
{
    QString qmsg = ui->LE_tcp_send_test->text();
    std::string msg = qmsg.toStdString();
    core_->sendRaw(msg);
}


void LoginWidget::handle(const EvUnknow& e) {
    ui->TextEdit_tcp_test_recv->setPlainText(QString::fromStdString(e.error_msg.message));
}

void LoginWidget::handle(const EvLoginOk& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("Login success");
}

void LoginWidget::handle(const EvLoginFail& e) {
    ui->TextEdit_tcp_test_recv->setPlainText(
        "Login failed: " + QString::fromStdString(e.resp.message));
}

void LoginWidget::handle(const EvOnlineUsers& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("Broadcast");
}

void LoginWidget::handle(const EvCmdConnect& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("EvCmdConnect");
}

void LoginWidget::handle(const EvCmdDisconnect& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("EvCmdDisconnect");

}

void LoginWidget::handle(const EvCmdLogin& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("EvCmdLogin");
}


void LoginWidget::handle(const EvTcpConnected& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("EvTcpConnected");
}

void LoginWidget::handle(const EvTcpDisconnected& e) {
    ui->TextEdit_tcp_test_recv->setPlainText("EvTcpDisconnected");
}




void LoginWidget::on_Bt_Login_clicked()
{
    QString qstr_user_name = ui->LE_UserName->text();
    std::string msg_user_name = qstr_user_name.toStdString();

    QString qstr_user_pass = ui->LE_UserPass->text();
    std::string msg_user_pass = qstr_user_pass.toStdString();

    core_->postEvent(EvCmdLogin{msg_user_name, msg_user_pass});
}

