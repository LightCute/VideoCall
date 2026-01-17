//AppWindow.h
#pragma once
#include <QWidget>
#include <QStackedWidget>
#include "memory"
#include "LoginWidget.h"
#include "widget.h"
#include "memory"

class ClientCore;

class AppWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AppWindow(QWidget *parent = nullptr);
    ~AppWindow() override;
private:
    QStackedWidget *stack;
    LoginWidget *loginPage;
    Widget *mainPage;

    // 核心：AppWindow 唯一持有 ClientCore 实例
    std::unique_ptr<ClientCore> core_;

    enum PageIndex {
        PAGE_LOGIN = 0,
        PAGE_MAIN
    };
};
