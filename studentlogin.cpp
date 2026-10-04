#include "studentlogin.h"
#include "ui_studentlogin.h"
#include "studentdashboard.h"

#include <QMessageBox>
#include <QLineEdit>

StudentLogin::StudentLogin(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StudentLogin)
{
    ui->setupUi(this);

    // Hide password
    ui->txtPassword->setEchoMode(QLineEdit::Password);

    // Login button
    connect(ui->btnLogin,
            &QPushButton::clicked,
            this,
            &StudentLogin::onLoginClicked);

    // Back button
    connect(ui->btnBack,
            &QPushButton::clicked,
            this,
            &StudentLogin::onBackClicked);
}


StudentLogin::~StudentLogin()
{
    delete ui;
}


// ==========================================
// STUDENT LOGIN
// ==========================================

void StudentLogin::onLoginClicked()
{
    QString studentID =
        ui->txtStudentID->text().trimmed();

    QString password =
        ui->txtPassword->text();


    if (studentID.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Please enter your Student ID and Password."
            );

        return;
    }


    // TEMPORARY LOGIN
    if (studentID == "249188" && password == "1234")
    {
        StudentDashboard dashboard(this);

        dashboard.exec();
    }
    else
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Invalid Student ID or Password."
            );
    }
}


// ==========================================
// BACK
// ==========================================

void StudentLogin::onBackClicked()
{
    close();
}