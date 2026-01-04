#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>
#include "ClientLoginProtocol.h"
#include <QThread>
using namespace proto;

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    cmdSocket_ = new CommandSocket();

    // 设置回调函数，线程安全调用 Qt UI
    cmdSocket_->setMessageCallback([this](const std::string& msg){

        ClientEvent event = ClientEventFactory::makeEvent(msg);

        QMetaObject::invokeMethod(this, [this, event]() {
            handleEvent(event);
        }, Qt::QueuedConnection);
    });
    // 作为客户端连接服务器
    //cmdSocket_->connectToServer("127.0.0.1", 6000);
    // 或者作为服务器：
    //cmdSocket_->startServer(6000);


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

    if(cmdSocket_->connectToServer("127.0.0.1", 6001) == true)
    {
        ui->TextEdit_tcp_test_recv->setPlainText("Connect sucess\n");
    }
    else
    {
        ui->TextEdit_tcp_test_recv->setPlainText("Connect fail\n");
    }
}


void LoginWidget::on_Bt_tcp_test_send_clicked()
{
    QString qmsg = ui->LE_tcp_send_test->text();
    std::string msg = qmsg.toStdString();
    cmdSocket_->sendMessage( msg);
}

void LoginWidget::handleEvent(const ClientEvent& e) {
    switch (e.type) {
    case ClientEventType::LoginOk:
        ui->TextEdit_tcp_test_recv->setPlainText("Login success");
        break;

    case ClientEventType::LoginFail:
        ui->TextEdit_tcp_test_recv->setPlainText(
            "Login failed: " + QString::fromStdString(e.loginResp.message));
        break;

    case ClientEventType::OnlineUsers:
        //updateOnlineList(e.onlineUsers);
        ui->TextEdit_tcp_test_recv->setPlainText("Broadcast");
        break;

    default:
        qDebug() << "Unknown event";
    }
}
