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

    // 所有参数补充 core:: 前缀
    void handle(const core::OutLoginFail&);
    void handle(const core::OutDisconnected&) ;
    void handle(const core::OutLoginOk&) ;
    void handle(const core::OutStateChanged& e) ;
    void handle(const core::OutConnect&);       // 新增
    void handle(const core::OutSendLogin&);     // 新增
};

#endif // LOGINWIDGET_H
