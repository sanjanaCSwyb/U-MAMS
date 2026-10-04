#include "adminlogin.h"
#include "ui_adminlogin.h"
#include "dashboard.h"

#include <QMessageBox>
#include <QLineEdit>

AdminLogin::AdminLogin(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AdminLogin)
    , passwordVisible(false)
{
    ui->setupUi(this);

    // Password field
    ui->txtPassword->setEchoMode(QLineEdit::Password);

    // Show / hide password button
    ui->btnShowPassword->setText("👁");

    connect(ui->btnLogin,
            &QPushButton::clicked,
            this,
            &AdminLogin::onLoginClicked);

    connect(ui->btnBack,
            &QPushButton::clicked,
            this,
            &AdminLogin::onBackClicked);

    connect(ui->btnShowPassword,
            &QToolButton::clicked,
            this,
            &AdminLogin::onShowPasswordClicked);
}

AdminLogin::~AdminLogin()
{
    delete ui;
}


// ===============================
// ADMIN LOGIN
// ===============================

void AdminLogin::onLoginClicked()
{
    QString username = ui->txtUsername->text().trimmed();
    QString password = ui->txtPassword->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Please enter your Admin Username and Password."
            );

        return;
    }

    // Temporary login credentials
    // Database authentication will be added later.
    if (username == "admin" && password == "admin123")
    {
        Dashboard *dashboard = new Dashboard();

        dashboard->setAttribute(Qt::WA_DeleteOnClose);

        dashboard->show();

        this->close();
    }
    else
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Invalid Admin Username or Password."
            );
    }
}


// ===============================
// BACK
// ===============================

void AdminLogin::onBackClicked()
{
    close();
}


// ===============================
// SHOW / HIDE PASSWORD
// ===============================

void AdminLogin::onShowPasswordClicked()
{
    passwordVisible = !passwordVisible;

    if (passwordVisible)
    {
        ui->txtPassword->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->txtPassword->setEchoMode(QLineEdit::Password);
    }
}