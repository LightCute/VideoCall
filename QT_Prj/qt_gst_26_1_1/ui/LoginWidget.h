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

    void onCoreOutput(const core::CoreOutput& out) override;

signals:
    void loginSuccess();
    void coreOutputReceived(const core::CoreOutput& out);

private slots:
    void on_Bt_Jump_Test_clicked();
    void on_Bt_ConnectToServer_clicked();
    void on_Bt_tcp_test_send_clicked();
    void on_Bt_Login_clicked();
    void handleCoreOutput(const core::CoreOutput& out);

private:
    Ui::LoginWidget *ui;
    ClientCore* core_;
    // 新增：UI 线程处理 Core 事件
    // 所有参数补充 core:: 前缀
    void handle(const core::OutLoginFail&);
    void handle(const core::OutDisconnected&) ;
    void handle(const core::OutLoginOk&) ;
    void handle(const core::OutStateChanged& e) ;
    void handle(const core::OutConnect&);       // 新增
    void handle(const core::OutSendLogin&);     // 新增

    void handle(const core::OutSendPing&);

    void handle(const core::OutUpdateAlive&);
    void handle(const core::OutOnlineUsers&);
};

#endif // LOGINWIDGET_H
