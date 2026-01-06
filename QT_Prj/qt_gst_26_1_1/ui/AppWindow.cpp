#include "AppWindow.h"
#include "LoginWidget.h"
#include "widget.h"
#include <QVBoxLayout>

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
{
    resize(900, 600);
    stack = new QStackedWidget(this);

    loginPage = new LoginWidget;
    mainPage  = new Widget;

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
