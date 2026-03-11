#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <memory>
#include <QTimer>
#include "../net/Websocket.h"       
#include "IUI.h"                    
#include "../core/IEventQueue.h"   


QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget , public IUI
{
    Q_OBJECT

public:
    Widget(IEventQueue* queue, QWidget *parent = nullptr);
    ~Widget();
    void showMessage(const std::string& msg) override;
private slots:

    void on_bt_connect_clicked();

private:
    Ui::Widget *ui;
    QTimer* m_timer;
    IEventQueue* m_queue;
};
#endif // WIDGET_H
