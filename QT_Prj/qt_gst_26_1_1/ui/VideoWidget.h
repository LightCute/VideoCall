//VideoWidget.h
#pragma once
#include <QWidget>
#include <QImage>

class VideoWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void setFrame(const QImage &img);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage frame_;
};
