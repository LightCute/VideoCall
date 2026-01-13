//AppWindow.h
#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "LoginWidget.h"
#include "widget.h"

class AppWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);

private:
    QStackedWidget *stack;
    LoginWidget *loginPage;
    Widget *mainPage;

    enum PageIndex {
        PAGE_LOGIN = 0,
        PAGE_MAIN
    };
};
