//QtCameraAdapter.h
#pragma once
#include <QObject>
#include "Frame.h"
#include "VideoWidget.h"

class QtCameraAdapter : public QObject {
    Q_OBJECT
public:
    explicit QtCameraAdapter(VideoWidget* view, QObject* parent = nullptr);

public slots:
    void onFrame(Frame frame);

private:
    VideoWidget* view_;
};
