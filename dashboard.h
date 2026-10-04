#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>

namespace Ui {
class Dashboard;
}

class Dashboard : public QDialog
{
    Q_OBJECT

public:
    explicit Dashboard(QWidget *parent = nullptr);
    ~Dashboard();

private slots:
    void onMedicalApplicationsClicked();
    void onViewApplicationsClicked();
    void onApproveDeclineClicked();
    void onNotificationsClicked();
    void onReportsClicked();
    void onChangePasswordClicked();
    void onLogoutClicked();

private:
    Ui::Dashboard *ui;
};

#endif // DASHBOARD_H