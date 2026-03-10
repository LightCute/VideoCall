#include "widget.h"
#include "ui_widget.h"
#include <memory>
#include <nlohmann/json.hpp>
#include "../core/core.h"
#include "../ui/IUI.h"
#include <iostream>
#include "../core/ConnectEvent.h"

Widget::Widget(IEventQueue* queue, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_timer(new QTimer(this))
    , m_queue(queue)
{
    ui->setupUi(this);
    

    connect(m_timer, &QTimer::timeout, [this]() {
        m_queue->process();
    });
    m_timer->start(10);
}

Widget::~Widget()
{
    delete ui;
}




void Widget::on_bt_connect_clicked()
{
    m_queue->enqueue(
        EventType::UI_NetConnectServer,
        std::make_shared<MessageEvent>(
            "ws://120.79.210.6:8000/abcd", false)
        );
}


void Widget::showMessage(const std::string& msg)
{

    ui->recvEdit->append(QString::fromStdString(msg));
}
