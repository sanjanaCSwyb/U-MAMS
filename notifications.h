#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QDialog>

class QScrollArea;

namespace Ui {
class Notifications;
}

class Notifications : public QDialog
{
    Q_OBJECT

public:
    explicit Notifications(QWidget *parent = nullptr);
    ~Notifications();

public slots:
    void refreshData();

private slots:
    void onBackClicked();

private:
    Ui::Notifications *ui;
    QScrollArea *notificationList = nullptr;
};

#endif // NOTIFICATIONS_H
