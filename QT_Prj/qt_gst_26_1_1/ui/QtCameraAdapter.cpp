//QtCameraAdapter.cpp
#include "QtCameraAdapter.h"
#include <QImage>

QtCameraAdapter::QtCameraAdapter(VideoWidget* view, QObject* parent)
    : QObject(parent), view_(view) {}

void QtCameraAdapter::onFrame(Frame f) {
    QImage img(
        f.data.data(),
        f.width,
        f.height,
        f.stride,
        QImage::Format_RGB888
        );

    view_->setFrame(img.copy());
}
