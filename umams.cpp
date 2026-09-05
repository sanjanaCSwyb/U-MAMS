#include "umams.h"
#include "ui_umams.h"
#include "dashboard.h"
#include <QMessageBox>

UMAMS::UMAMS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UMAMS)
{
    ui->setupUi(this);

    connect(ui->btnLogin, &QPushButton::clicked,
            this, &UMAMS::onLoginClicked);
}

UMAMS::~UMAMS()
{
    delete ui;
}

void UMAMS::onLoginClicked()
{
    QString username = ui->txtUsername->text();
    QString password = ui->txtPassword->text();

    if (username == "admin" && password == "1234")
    {
        // Create Dashboard window
        Dashboard *dashboard = new Dashboard(this);

        // Show Dashboard
        dashboard->show();

        // Hide Login window
        this->hide();
    }
    else
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Invalid username or password."
            );
    }
}