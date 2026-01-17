//main.cpp
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <variant>

#ifdef slots
#undef slots
#endif
#ifdef signals
#undef signals
#endif

#include <QApplication>
#include "widget.h"
#include "LoginWidget.h"
#include "AppWindow.h"
#include "core/CoreOutput.h"
// #include "widget.h"
// #include "LoginWidget.h"
// #include <QApplication>
// #include "AppWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Frame>("Frame");
    qRegisterMetaType<core::CoreOutput>("core::CoreOutput");  // 新增
    AppWindow w;
    //LoginWidget login_W;
    //login_W.show();
    w.show();
    return a.exec();
}
