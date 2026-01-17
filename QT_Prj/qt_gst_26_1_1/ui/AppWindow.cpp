// ui/AppWindow.cpp
#include "AppWindow.h"
#include "LoginWidget.h"
#include "widget.h"
#include "core/ClientCore.h"  // 包含 Core 头文件
#include <QVBoxLayout>

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
{
    resize(900, 600);
    stack = new QStackedWidget(this);

    // 核心：创建唯一的 ClientCore 实例（纯 C++，无 Qt 依赖）
    core_ = std::make_unique<ClientCore>();

    // 传递 Core 指针给子窗口
    loginPage = new LoginWidget(core_.get(), this);
    mainPage  = new Widget(core_.get(), this);

    stack->addWidget(loginPage); // index 0
    stack->addWidget(mainPage);  // index 1

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(stack);
    setLayout(layout);

    // 登录成功 → 切换页面
    connect(loginPage, &LoginWidget::loginSuccess, this, [=](){
        stack->setCurrentIndex(PAGE_MAIN);
    });

    // 初始页面
    stack->setCurrentIndex(PAGE_LOGIN);
}

// 析构：智能指针自动释放 Core
AppWindow::~AppWindow() = default;
