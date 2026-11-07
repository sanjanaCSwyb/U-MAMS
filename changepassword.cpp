#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "changepassword.h"
#include "ui_changepassword.h"

#include <QMessageBox>
#include <QLineEdit>
#include <QToolButton>

ChangePassword::ChangePassword(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangePassword)
    , currentPasswordVisible(false)
    , newPasswordVisible(false)
    , confirmPasswordVisible(false)
{
    ui->setupUi(this);
    if (!AppSession::protectAuthenticated(this)) return;

    // =========================================
    // PASSWORD FIELDS
    // =========================================

    ui->txtCurrentPassword->setEchoMode(QLineEdit::Password);
    ui->txtNewPassword->setEchoMode(QLineEdit::Password);
    ui->txtConfirmPassword->setEchoMode(QLineEdit::Password);


    // =========================================
    // EYE BUTTONS
    // =========================================

    ui->toolButtonCurrent->setText(QStringLiteral("\u25CF"));
    ui->toolButtonNew->setText(QStringLiteral("\u25CF"));
    ui->toolButtonConfirm->setText(QStringLiteral("\u25CF"));


    // =========================================
    // MAIN BUTTON CONNECTIONS
    // =========================================

    connect(ui->btnChangePassword,
            &QPushButton::clicked,
            this,
            &ChangePassword::onChangePasswordClicked);

    connect(ui->btnBack,
            &QPushButton::clicked,
            this,
            &ChangePassword::onBackClicked);


    // =========================================
    // EYE BUTTON CONNECTIONS
    // =========================================

    // Current Password eye
    connect(ui->toolButtonCurrent,
            &QToolButton::clicked,
            this,
            &ChangePassword::toggleCurrentPassword);

    // New Password eye
    connect(ui->toolButtonNew,
            &QToolButton::clicked,
            this,
            &ChangePassword::toggleNewPassword);

    // Confirm Password eye
    connect(ui->toolButtonConfirm,
            &QToolButton::clicked,
            this,
            &ChangePassword::toggleConfirmPassword);
    FigmaUi::install(this);
}


ChangePassword::~ChangePassword()
{
    delete ui;
}


// =========================================
// CHANGE PASSWORD
// =========================================

void ChangePassword::onChangePasswordClicked()
{
    if (!AppSession::authenticated()) return;
    QString currentPassword =
        ui->txtCurrentPassword->text();

    QString newPassword =
        ui->txtNewPassword->text();

    QString confirmPassword =
        ui->txtConfirmPassword->text();


    // Check empty fields
    if (currentPassword.isEmpty() ||
        newPassword.isEmpty() ||
        confirmPassword.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Change Password",
            "Please fill in all password fields."
            );

        return;
    }


    // Check password length
    if (newPassword.length() < 8)
    {
        QMessageBox::warning(
            this,
            "Change Password",
            "New password must contain at least 8 characters."
            );

        return;
    }


    // Check matching passwords
    if (newPassword != confirmPassword)
    {
        QMessageBox::warning(
            this,
            "Change Password",
            "New password and confirmation password do not match."
            );

        return;
    }


    ui->btnChangePassword->setEnabled(false);
    const bool saved = Database::changePassword(currentPassword, newPassword);
    ui->btnChangePassword->setEnabled(true);
    if (!saved) {
        QMessageBox::warning(
            this,
            tr("Change Password"),
            Database::error().isEmpty() ? tr("Unable to change your password.") : Database::error());
        return;
    }

    QMessageBox::information(
        this,
        "Password Changed",
        "Your password has been changed successfully."
        );

    ui->txtCurrentPassword->clear();
    ui->txtNewPassword->clear();
    ui->txtConfirmPassword->clear();

    close();
}


// =========================================
// BACK
// =========================================

void ChangePassword::onBackClicked()
{
    close();
}


// =========================================
// SHOW / HIDE CURRENT PASSWORD
// =========================================

void ChangePassword::toggleCurrentPassword()
{
    if (!AppSession::authenticated()) return;
    currentPasswordVisible = !currentPasswordVisible;

    if (currentPasswordVisible)
    {
        ui->txtCurrentPassword->setEchoMode(
            QLineEdit::Normal
            );
    }
    else
    {
        ui->txtCurrentPassword->setEchoMode(
            QLineEdit::Password
            );
    }
}


// =========================================
// SHOW / HIDE NEW PASSWORD
// =========================================

void ChangePassword::toggleNewPassword()
{
    if (!AppSession::authenticated()) return;
    newPasswordVisible = !newPasswordVisible;

    if (newPasswordVisible)
    {
        ui->txtNewPassword->setEchoMode(
            QLineEdit::Normal
            );
    }
    else
    {
        ui->txtNewPassword->setEchoMode(
            QLineEdit::Password
            );
    }
}


// =========================================
// SHOW / HIDE CONFIRM PASSWORD
// =========================================

void ChangePassword::toggleConfirmPassword()
{
    if (!AppSession::authenticated()) return;
    confirmPasswordVisible = !confirmPasswordVisible;

    if (confirmPasswordVisible)
    {
        ui->txtConfirmPassword->setEchoMode(
            QLineEdit::Normal
            );
    }
    else
    {
        ui->txtConfirmPassword->setEchoMode(
            QLineEdit::Password
            );
    }
}
