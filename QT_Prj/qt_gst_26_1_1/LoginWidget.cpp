#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>
#include "ClientLoginProtocol.h"
#include <QThread>
LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    cmdSocket_ = new CommandSocket();

    // 设置回调函数，线程安全调用 Qt UI
    cmdSocket_->setMessageCallback([this](const std::string& msg){
        ClientLoginProtocol::LoginResponse resp;
        auto type = ClientLoginProtocol::parseLoginResponse(msg, resp);

        qDebug() << "current thread:" << QThread::currentThread();
        qDebug() << "ui thread:" << qApp->thread();
        QMetaObject::invokeMethod(this, [this, type, resp](){

            if (type == ClientLoginProtocol::ResponseType::LOGIN_OK) {
                ui->TextEdit_tcp_test_recv->setPlainText("Login success");
                //emit loginSuccess();
            }
            else if (type == ClientLoginProtocol::ResponseType::LOGIN_FAIL) {
                ui->TextEdit_tcp_test_recv->setPlainText(
                    "Login failed: " + QString::fromStdString(resp.message)
                    );
                qDebug() << "current thread:" << QThread::currentThread();
                qDebug() << "ui thread:" << qApp->thread();
            }
            else if (type == ClientLoginProtocol::ResponseType::ONLINE_USERS) {
                ui->TextEdit_tcp_test_recv->setPlainText(
                    "Online users: " + QString::fromStdString(resp.message)
                    );
            }
            else {
                // 其他消息，调试用
                ui->TextEdit_tcp_test_recv->setPlainText(
                    QString::fromStdString("Unknown: " + resp.message)
                    );
            };
            Qt::QueuedConnection;   // ⭐ 关键！
        });
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
    cmdSocket_->sendMessage(msg);
}

