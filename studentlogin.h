#ifndef STUDENTLOGIN_H
#define STUDENTLOGIN_H

#include <QDialog>
#include "app_session.h"

namespace Ui
{
class StudentLogin;
}

class StudentLogin : public QDialog
{
    Q_OBJECT

public:

    explicit StudentLogin(QWidget *parent = nullptr);

    ~StudentLogin();

    AppSession::Identity authenticatedIdentity() const { return authenticatedUser; }

private slots:

    void onLoginClicked();

    void onBackClicked();

    void onRegisterClicked();

private:

    Ui::StudentLogin *ui;
    AppSession::Identity authenticatedUser;
    bool authenticationInProgress = false;
    bool registrationInProgress = false;
};

#endif // STUDENTLOGIN_H
