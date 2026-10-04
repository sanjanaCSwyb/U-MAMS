#ifndef MYAPPLICATIONS_H
#define MYAPPLICATIONS_H

#include <QDialog>

namespace Ui {
class MyApplications;
}

class MyApplications : public QDialog
{
    Q_OBJECT

public:
    explicit MyApplications(QWidget *parent = nullptr);
    ~MyApplications();

private slots:
    void onViewDetailsClicked();
    void onBackClicked();

private:
    Ui::MyApplications *ui;
};

#endif // MYAPPLICATIONS_H