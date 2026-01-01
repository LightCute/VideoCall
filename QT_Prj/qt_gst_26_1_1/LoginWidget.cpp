#include "LoginWidget.h"
#include "ui_LoginWidget.h"
#include <QMessageBox>
#include <QMetaObject>
#include <QDebug>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);



}

LoginWidget::~LoginWidget()
{
    delete ui;
}
