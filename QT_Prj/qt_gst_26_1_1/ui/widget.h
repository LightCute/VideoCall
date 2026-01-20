// ui/widget.h（完整修复版）
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "QtCameraAdapter.h"
#include "CameraManager.h"
#include "core/ICoreListener.h"
#include "core/CoreOutput.h"

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
    void onCoreOutput(const core::CoreOutput& out) override;

signals:
    void coreOutputReceived(const core::CoreOutput& out);

private slots:
    void on_Bt_video_on_off_clicked();
    void on_Bt_video_off_clicked();
    void on_Bt_tcp_send_clicked();
    void handleCoreOutput(const core::CoreOutput& out); // 槽函数声明

private:
    Ui::Widget *ui;
    ClientCore* core_; // 新增：Core指针成员
    VideoWidget *video_;
    CameraManager camera_;
    QtCameraAdapter* adapter_;



    void handle(const core::OutDisconnected&);

    void handle(const core::OutConnect&);

    void handle(const core::OutSendLogin&);

    void handle(const core::OutSendPing&);

    void handle(const core::OutUpdateAlive&);

    void handle(const core::OutLoginOk&);

    void handle(const core::OutLoginFail&);

    void handle(const core::OutOnlineUsers&);

    void handle(const core::OutStateChanged&);

    void handle(const core::OutSelectLan&) ;

    void handle(const core::OutSelectVpn&) ;
    void handle(const core::OutSendText& e) ;

    void handle(const core::OutForwardText& e) ;

};
#endif // WIDGET_H
