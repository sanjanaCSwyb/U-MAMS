#ifndef ADMINLOGIN_H
#define ADMINLOGIN_H

#include <QDialog>

namespace Ui {
class AdminLogin;
}

class AdminLogin : public QDialog
{
    Q_OBJECT

public:
    explicit AdminLogin(QWidget *parent = nullptr);
    ~AdminLogin();

private slots:
    void onLoginClicked();
    void onBackClicked();
    void onShowPasswordClicked();

private:
    Ui::AdminLogin *ui;
    bool passwordVisible;
};

#endif