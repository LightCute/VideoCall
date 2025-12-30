#include "VideoWidget.h"
#include <QPainter>

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
    if (!frame_.isNull()) {
        p.drawImage(rect(), frame_);
    } else {
        p.fillRect(rect(), Qt::black);
    }
}
