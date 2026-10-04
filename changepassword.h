#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>

namespace Ui {
class ChangePassword;
}

class ChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget *parent = nullptr);
    ~ChangePassword();

private slots:
    void onChangePasswordClicked();
    void onBackClicked();

    void toggleCurrentPassword();
    void toggleNewPassword();
    void toggleConfirmPassword();

private:
    Ui::ChangePassword *ui;

    bool currentPasswordVisible;
    bool newPasswordVisible;
    bool confirmPasswordVisible;
};

#endif