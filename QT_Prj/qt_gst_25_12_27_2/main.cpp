#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<Frame>("Frame");
    Widget w;
    w.show();
    return a.exec();
}
