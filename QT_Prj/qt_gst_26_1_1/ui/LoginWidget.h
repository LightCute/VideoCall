//LoginWidget.h
#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "core/ICoreListener.h"  // 实现抽象接口
#include "core/CoreOutput.h"     // Core 输出事件

class ClientCore;

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget, public core::ICoreListener
{
    Q_OBJECT

public:
    explicit LoginWidget(ClientCore* core, QWidget* parent = nullptr);
    ~LoginWidget() override;

    void onUiOutput(const core::UiOutput& out) override;

signals:
    void loginSuccess();
    void coreOutputReceived(const core::UiOutput& out);

private slots:
    void on_Bt_Jump_Test_clicked();
    void on_Bt_ConnectToServer_clicked();
    void on_Bt_tcp_test_send_clicked();
    void on_Bt_Login_clicked();
    void handleCoreOutput(const core::UiOutput& out);

private:
    Ui::LoginWidget *ui;
    ClientCore* core_;

    void handle(const core::UiOutStateChanged& e);
    void handle(const core::UiOutLoginOk&);
    void handle(const core::UiOutLoginFail& e);
    void handle(const core::UiOutDisconnected&);
    void handle(const core::UiOutOnlineUsers&);
    void handle(const core::UiOutForwardText& e);

};

#endif // LOGINWIDGET_H
