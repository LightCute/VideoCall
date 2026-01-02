#include "widget.h"
#include "LoginWidget.h"
#include <QApplication>
#include "AppWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Frame>("Frame");
    AppWindow w;
    //LoginWidget login_W;
    //login_W.show();
    w.show();
    return a.exec();
}
