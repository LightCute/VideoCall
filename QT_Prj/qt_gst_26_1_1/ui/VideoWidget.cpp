#include "VideoWidget.h"
#include <QPainter>
#include <QDebug>
VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent) {
    setMinimumSize(640, 480);
}

void VideoWidget::setFrame(const QImage &img) {
    frame_ = img;
    update();
}

void VideoWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (frame_.isNull())
        return;

    QSize imgSize = frame_.size();
    imgSize.scale(size(), Qt::KeepAspectRatio);

    QRect targetRect(
        (width()  - imgSize.width())  / 2,
        (height() - imgSize.height()) / 2,
        imgSize.width(),
        imgSize.height()
        );
    //qDebug() << "widget size:" << size();

    p.drawImage(targetRect, frame_);
}

