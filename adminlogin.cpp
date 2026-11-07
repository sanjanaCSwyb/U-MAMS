#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "adminlogin.h"
#include "ui_adminlogin.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QScopedValueRollback>


AdminLogin::AdminLogin(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminLogin)
    , passwordVisible(false)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - Admin Login");


    // =========================================================
    // PASSWORD SETTINGS
    // =========================================================

    ui->txtPassword->setEchoMode(QLineEdit::Password);

    // Eye button
    ui->btnShowPassword->setText(QStringLiteral("\u25CF"));
    ui->btnShowPassword->setToolTip("Show password");

    // =========================================================
    // BUTTON CONNECTIONS
    // =========================================================

    connect(
        ui->btnLogin,
        &QPushButton::clicked,
        this,
        &AdminLogin::onLoginClicked
        );

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &AdminLogin::onBackClicked
        );

    connect(
        ui->btnShowPassword,
        &QToolButton::clicked,
        this,
        &AdminLogin::onShowPasswordClicked
        );
    FigmaUi::install(this);
}


AdminLogin::~AdminLogin()
{
    delete ui;
}


// =========================================================
// ADMIN LOGIN
// =========================================================

void AdminLogin::onLoginClicked()
{
    if (authenticationInProgress) return;
    QScopedValueRollback<bool> authenticating(authenticationInProgress, true);
    authenticatedUser = {};
    QString username =
        ui->txtUsername->text().trimmed();

    QString password =
        ui->txtPassword->text();


    // Check empty fields
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Please enter your Admin Username and Password."
            );

        return;
    }


    AppSession::Identity user;
    ui->btnLogin->setEnabled(false);
    const bool authenticated =
        Database::authenticate(username, password, AppSession::Role::Admin, user);
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
                ? tr("Unable to sign in with this admin account.")
                : Database::error()
            );
        ui->txtPassword->clear();
        ui->txtPassword->setFocus();
    }
}


// =========================================================
// BACK BUTTON
// =========================================================

void AdminLogin::onBackClicked()
{
    ui->txtPassword->clear();
    authenticatedUser = {};
    done(AppSession::LoginBackToPortal);
}


// =========================================================
// PASSWORD VISIBILITY
// =========================================================

void AdminLogin::onShowPasswordClicked()
{
    passwordVisible = !passwordVisible;

    if (passwordVisible)
    {
        ui->txtPassword->setEchoMode(QLineEdit::Normal);

        ui->btnShowPassword->setText(
            QStringLiteral("\u25CF")
            );

        ui->btnShowPassword->setToolTip("Hide password");
    }
    else
    {
        ui->txtPassword->setEchoMode(QLineEdit::Password);

        ui->btnShowPassword->setText(
            QStringLiteral("\u25CF")
            );

        ui->btnShowPassword->setToolTip("Show password");
    }
}
