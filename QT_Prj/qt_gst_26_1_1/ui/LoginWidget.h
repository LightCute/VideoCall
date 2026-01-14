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

    void handle(const OutLoginFail&);

    void handle(const OutDisconnected&) ;

    void handle(const OutLoginOk&) ;

    void handle(const OutStateChanged& e) ;

    void handle(const OutConnect&);       // 新增
    void handle(const OutSendLogin&);

};

#endif // LOGINWIDGET_H
