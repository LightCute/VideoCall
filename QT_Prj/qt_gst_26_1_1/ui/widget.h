// ui/widget.h（完整修复版）
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QtCameraAdapter.h"
#include "CameraManager.h"
#include "core/ICoreListener.h"
#include "core/CoreOutput.h"
#include "media/VideoReceiver.h"
// 前置声明（减少耦合）
class ClientCore;

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget, public core::ICoreListener
{
    Q_OBJECT

public:
    explicit Widget(ClientCore* core, QWidget *parent = nullptr);
    ~Widget() override;

    // 仅实现ICoreListener要求的纯虚函数
    void onUiOutput(const core::UiOutput& out) override;

signals:
    void coreOutputReceived(const core::UiOutput& out);

private slots:
    void on_Bt_video_on_off_clicked();
    void on_Bt_video_off_clicked();
    void on_Bt_tcp_send_clicked();
    void handleCoreOutput(const core::UiOutput& out); // 槽函数声明

    void on_Bt_Call_clicked();

    void on_Bt_AcceptCall_clicked();

    void on_Bt_RejectCall_clicked();

    void on_Bt_set_lan_clicked();

    void on_Bt_sen_vpn_clicked();

    void on_Bt_Hangup_clicked();

private:
    Ui::Widget *ui;
    ClientCore* core_; // 新增：Core指针成员
    VideoWidget *video_;
    CameraManager camera_;
    QtCameraAdapter* adapter_;
    VideoWidget *remote_video_;
    VideoReceiver receiver_;
    QtCameraAdapter* remote_adapter_;

    void handle(const core::UiOutStateChanged&);
    void handle(const core::UiOutLoginOk&);
    void handle(const core::UiOutLoginFail&);
    void handle(const core::UiOutDisconnected&);
    void handle(const core::UiOutOnlineUsers&);
    void handle(const core::UiOutForwardText& e);
    void handle(const core::UiOutShowIncomingCall& e);
    void handle(const core::UiOutMediaReadyFinal& e);
    void handle(const core::UiOutCallEnded& e);
    void handle(const core::UiOutStopMedia&);

};
#endif // WIDGET_H
