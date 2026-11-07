#ifndef ADMINLOGIN_H
#define ADMINLOGIN_H

#include <QDialog>
#include "app_session.h"


namespace Ui
{
class AdminLogin;
}


class AdminLogin : public QDialog
{
    Q_OBJECT

public:

    explicit AdminLogin(QWidget *parent = nullptr);

    ~AdminLogin();

    AppSession::Identity authenticatedIdentity() const { return authenticatedUser; }


private slots:

    void onLoginClicked();

    void onBackClicked();

    void onShowPasswordClicked();


private:

    Ui::AdminLogin *ui;

    bool passwordVisible;
    AppSession::Identity authenticatedUser;
    bool authenticationInProgress = false;
};

#endif // ADMINLOGIN_H
