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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *Bt_video_on_off;
    QPushButton *Bt_video_off;
    QWidget *videoContainer;
    QTextEdit *Text_tcp_recv_text;
    QPushButton *Bt_tcp_send;
    QLineEdit *lineEdit_tcp_send;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 600);
        Bt_video_on_off = new QPushButton(Widget);
        Bt_video_on_off->setObjectName(QString::fromUtf8("Bt_video_on_off"));
        Bt_video_on_off->setGeometry(QRect(230, 540, 88, 26));
        Bt_video_off = new QPushButton(Widget);
        Bt_video_off->setObjectName(QString::fromUtf8("Bt_video_off"));
        Bt_video_off->setGeometry(QRect(370, 540, 88, 26));
        videoContainer = new QWidget(Widget);
        videoContainer->setObjectName(QString::fromUtf8("videoContainer"));
        videoContainer->setGeometry(QRect(70, 10, 640, 480));
        Text_tcp_recv_text = new QTextEdit(Widget);
        Text_tcp_recv_text->setObjectName(QString::fromUtf8("Text_tcp_recv_text"));
        Text_tcp_recv_text->setGeometry(QRect(33, 529, 151, 41));
        Bt_tcp_send = new QPushButton(Widget);
        Bt_tcp_send->setObjectName(QString::fromUtf8("Bt_tcp_send"));
        Bt_tcp_send->setGeometry(QRect(520, 540, 88, 26));
        lineEdit_tcp_send = new QLineEdit(Widget);
        lineEdit_tcp_send->setObjectName(QString::fromUtf8("lineEdit_tcp_send"));
        lineEdit_tcp_send->setGeometry(QRect(650, 540, 113, 26));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        Bt_video_on_off->setText(QCoreApplication::translate("Widget", "ON", nullptr));
        Bt_video_off->setText(QCoreApplication::translate("Widget", "OFF", nullptr));
        Bt_tcp_send->setText(QCoreApplication::translate("Widget", "Send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
