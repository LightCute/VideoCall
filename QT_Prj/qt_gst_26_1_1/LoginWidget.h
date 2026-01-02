#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "widget.h"
#include "CommandSocket.h"

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

private:
    Ui::LoginWidget *ui;
    CommandSocket *cmdSocket_ ;

};

#endif // LOGINWIDGET_H
