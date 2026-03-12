#ifndef QT_UI_H
#define QT_UI_H

#include <QWidget>
#include "../service/abstract_ui.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class Qt_UI;
}
QT_END_NAMESPACE

class Qt_UI : public QWidget , public AbstractUI
{
    Q_OBJECT

signals:
    void signal_updateMessage(const QString& msg); 

public:
    Qt_UI(QWidget *parent = nullptr);
    ~Qt_UI();
    void showMessage(const std::string& message) override;
    void showUI() override;
private slots:
    void on_bt_connect_clicked();
    void slot_updateMessage(const QString& msg);
private:
    Ui::Qt_UI *ui;
};

#endif // QT_UI_H
