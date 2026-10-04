#ifndef STUDENTPROFILE_H
#define STUDENTPROFILE_H

#include <QDialog>

namespace Ui {
class StudentProfile;
}

class StudentProfile : public QDialog
{
    Q_OBJECT

public:
    explicit StudentProfile(QWidget *parent = nullptr);
    ~StudentProfile();

private slots:
    void onChangePasswordClicked();
    void onBackClicked();

private:
    Ui::StudentProfile *ui;
};

#endif