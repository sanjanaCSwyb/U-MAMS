#ifndef APPROVEDECLINE_H
#define APPROVEDECLINE_H

#include <QDialog>

namespace Ui {
class ApproveDecline;
}

class ApproveDecline : public QDialog
{
    Q_OBJECT

public:
    explicit ApproveDecline(QWidget *parent = nullptr);
    ~ApproveDecline();

private slots:
    void onApproveClicked();
    void onDeclineClicked();
    void onBackClicked();

private:
    Ui::ApproveDecline *ui;
};

#endif // APPROVEDECLINE_H