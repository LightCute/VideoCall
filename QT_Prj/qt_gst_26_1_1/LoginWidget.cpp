#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>


LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    cmdSocket_ = new CommandSocket();

    // 设置回调函数，线程安全调用 Qt UI
    cmdSocket_->setMessageCallback([this](const std::string& msg){
        QString qmsg = QString::fromStdString(msg);
        // 线程安全调用 Qt UI
        QMetaObject::invokeMethod(this, [this, qmsg](){
            ui->TextEdit_tcp_test_recv->setPlainText(qmsg); // 假设有 textEdit 显示消息
            //qDebug("%s\n",qmsg);
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
    if(cmdSocket_->connectToServer("127.0.0.1", 6000) == true)
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

