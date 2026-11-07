#ifndef MONTHLYAPPROVEDMEDICALS_H
#define MONTHLYAPPROVEDMEDICALS_H

#include <QDialog>

namespace Ui { class MonthlyApprovedMedicals; }

class MonthlyApprovedMedicals final : public QDialog
{
    Q_OBJECT

public:
    explicit MonthlyApprovedMedicals(QWidget *parent = nullptr);
    ~MonthlyApprovedMedicals() override;

public slots:
    void refreshData();

private slots:
    void queueSelectedDetails();
    void openSelectedDetails();
    void updateSelection();

private:
    Ui::MonthlyApprovedMedicals *ui;
    bool loading = false;
    bool detailsQueued = false;
    bool detailsOpen = false;
};

#endif // MONTHLYAPPROVEDMEDICALS_H
