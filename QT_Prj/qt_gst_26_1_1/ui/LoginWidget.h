//LoginWidget.h
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "widget.h"

#include "ClientCore.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginSuccess();

private slots:

    void on_Bt_Jump_Test_clicked();

    void on_Bt_ConnectToServer_clicked();

    void on_Bt_tcp_test_send_clicked();

    void on_Bt_Login_clicked();

private:
    Ui::LoginWidget *ui;
    ClientCore* core_;

    void handle(const EvUnknow& e) ;

    void handle(const EvLoginOk& e) ;

    void handle(const EvLoginFail& e);

    void handle(const EvOnlineUsers& e);

    void handle(const EvCmdConnect& e) ;

    void handle(const EvCmdDisconnect& e) ;

    void handle(const EvCmdLogin& e) ;

    void handle(const EvTcpConnected& e) ;

    void handle(const EvTcpDisconnected& e);



};

#endif // LOGINWIDGET_H
