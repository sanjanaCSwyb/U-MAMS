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

    // =========================================
    // PASSWORD FIELDS
    // =========================================

    ui->txtCurrentPassword->setEchoMode(QLineEdit::Password);
    ui->txtNewPassword->setEchoMode(QLineEdit::Password);
    ui->txtConfirmPassword->setEchoMode(QLineEdit::Password);


    // =========================================
    // EYE BUTTONS
    // =========================================

    ui->toolButtonCurrent->setText("👁");
    ui->toolButtonNew->setText("👁");
    ui->toolButtonConfirm->setText("👁");


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


    // Temporary current password
    // Database authentication will be added later.
    if (currentPassword != "1234")
    {
        QMessageBox::warning(
            this,
            "Change Password",
            "Current password is incorrect."
            );

        return;
    }


    // Check password length
    if (newPassword.length() < 4)
    {
        QMessageBox::warning(
            this,
            "Change Password",
            "New password must contain at least 4 characters."
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


    // Temporary success
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