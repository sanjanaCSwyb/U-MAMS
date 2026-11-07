#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "studentlogin.h"
#include "ui_studentlogin.h"
#include "studentregistration.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QScopedValueRollback>


StudentLogin::StudentLogin(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StudentLogin)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - Student Login");

    /*
     * =====================================================
     * PASSWORD FIELD
     * =====================================================
     */

    ui->txtPassword->setEchoMode(QLineEdit::Password);

    ui->btnShowPassword->setToolTip(tr("Show password"));
    ui->btnShowPassword->setAccessibleName(tr("Show password"));
    connect(ui->btnShowPassword, &QToolButton::clicked, this, [this]() {
        const bool showPassword =
            ui->txtPassword->echoMode() == QLineEdit::Password;
        ui->txtPassword->setEchoMode(
            showPassword ? QLineEdit::Normal : QLineEdit::Password);
        const QString action = showPassword ? tr("Hide password") : tr("Show password");
        ui->btnShowPassword->setToolTip(action);
        ui->btnShowPassword->setAccessibleName(action);
    });


    /*
     * =====================================================
     * BUTTON CONNECTIONS
     * =====================================================
     */

    connect(
        ui->btnLogin,
        &QPushButton::clicked,
        this,
        &StudentLogin::onLoginClicked
        );

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &StudentLogin::onBackClicked
        );
    connect(ui->btnRegister, &QPushButton::clicked,
            this, &StudentLogin::onRegisterClicked);
    FigmaUi::install(this);
}


StudentLogin::~StudentLogin()
{
    delete ui;
}


/*
 * =========================================================
 * LOGIN
 * =========================================================
 */

void StudentLogin::onLoginClicked()
{
    if (authenticationInProgress || registrationInProgress) return;
    QScopedValueRollback<bool> authenticating(authenticationInProgress, true);
    authenticatedUser = {};
    QString studentID =
        ui->txtStudentID->text().trimmed();

    QString password =
        ui->txtPassword->text();


    /*
     * =====================================================
     * EMPTY FIELD CHECK
     * =====================================================
     */

    if (studentID.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Please enter your Student ID and Password."
            );

        return;
    }


    AppSession::Identity user;
    ui->btnLogin->setEnabled(false);
    const bool authenticated =
        Database::authenticate(studentID, password, AppSession::Role::Student, user);
    ui->btnLogin->setEnabled(true);

    if (authenticated)
    {
        authenticatedUser = user;
        ui->txtPassword->clear();
        done(AppSession::LoginAuthenticated);
    }
    else
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            Database::error().isEmpty()
                ? tr("Unable to sign in with this student account.")
                : Database::error()
            );
        ui->txtPassword->clear();
        ui->txtPassword->setFocus();
    }
}


/*
 * =========================================================
 * BACK BUTTON
 * =========================================================
 */

void StudentLogin::onBackClicked()
{
    ui->txtPassword->clear();
    authenticatedUser = {};
    done(AppSession::LoginBackToPortal);
}

void StudentLogin::onRegisterClicked()
{
    if (authenticationInProgress || registrationInProgress || AppSession::authenticated()) return;
    QScopedValueRollback<bool> registering(registrationInProgress, true);
    StudentRegistration registration(this);
    if (registration.exec() != QDialog::Accepted) return;

    ui->txtStudentID->setText(registration.registeredNumber());
    ui->txtPassword->clear();
    ui->txtPassword->setEchoMode(QLineEdit::Password);
    ui->btnShowPassword->setToolTip(tr("Show password"));
    ui->btnShowPassword->setAccessibleName(tr("Show password"));
    ui->txtPassword->setFocus();
}
