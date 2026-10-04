#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void onSearchClicked();
    void onStatusChanged(int index);
    void onApproveClicked();
    void onDeclineClicked();
    void onViewDetailsClicked();
    void onBackClicked();
private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
