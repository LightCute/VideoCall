#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "socket.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(WebSocket* ws, QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_bt_connect2Server_clicked();

    void on_bt_call_clicked();

private:
    Ui::Widget *ui;
    WebSocket* m_ws;
};
#endif // WIDGET_H
