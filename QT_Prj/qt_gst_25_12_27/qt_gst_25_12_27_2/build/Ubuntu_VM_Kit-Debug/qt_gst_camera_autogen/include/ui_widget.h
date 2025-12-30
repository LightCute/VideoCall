/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *Bt_video_on_off;
    QPushButton *Bt_video_off;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 600);
        Bt_video_on_off = new QPushButton(Widget);
        Bt_video_on_off->setObjectName(QString::fromUtf8("Bt_video_on_off"));
        Bt_video_on_off->setGeometry(QRect(670, 490, 88, 26));
        Bt_video_off = new QPushButton(Widget);
        Bt_video_off->setObjectName(QString::fromUtf8("Bt_video_off"));
        Bt_video_off->setGeometry(QRect(670, 540, 88, 26));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        Bt_video_on_off->setText(QCoreApplication::translate("Widget", "ON", nullptr));
        Bt_video_off->setText(QCoreApplication::translate("Widget", "OFF", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
