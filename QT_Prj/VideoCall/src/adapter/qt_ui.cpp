#include <string>
#include "qt_ui.h"
#include "ui_qt_ui.h"
#include "../framework/event/event_bus.h"
#include "../business/connect/event/connect_server_event.h"
Qt_UI::Qt_UI(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Qt_UI)
{
    ui->setupUi(this);
    connect(this, &Qt_UI::signal_updateMessage, 
            this, &Qt_UI::slot_updateMessage,
            Qt::QueuedConnection);
}

Qt_UI::~Qt_UI()
{
    delete ui;
}

void Qt_UI::showMessage(const std::string &message){
    QString qMsg = QString::fromStdString(message);
    emit signal_updateMessage(qMsg);
}

void Qt_UI::slot_updateMessage(const QString &msg) {
    ui->text_edit_showMsg->setText(msg);
}

void Qt_UI::showUI() {
    this->show(); // 调用 QWidget 自带的 show() 方法
}
void Qt_UI::on_bt_connect_clicked()
{
    EventBus::GetInstance().publish(
        std::make_unique<ConnectServerEvent>("ws://120.79.210.6:8000/abcd")
        );
}

