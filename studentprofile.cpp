#include "studentprofile.h"
#include "ui_studentprofile.h"
#include "changepassword.h"
#include <QMessageBox>

StudentProfile::StudentProfile(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StudentProfile)
{
    ui->setupUi(this);

    // Temporary student information
    ui->txtStudentID->setText("249188");
    ui->txtStudentName->setText("Kasun");
    ui->txtEmail->setText("student@university.edu");
    ui->txtDepartment->setText(
        "Material and Nano Science Department"
        );

    // Make profile information read-only
    ui->txtStudentID->setReadOnly(true);
    ui->txtStudentName->setReadOnly(true);
    ui->txtEmail->setReadOnly(true);
    ui->txtDepartment->setReadOnly(true);

    connect(ui->btnChangePassword,
            &QPushButton::clicked,
            this,
            &StudentProfile::onChangePasswordClicked);

    connect(ui->btnBack,
            &QPushButton::clicked,
            this,
            &StudentProfile::onBackClicked);
}

StudentProfile::~StudentProfile()
{
    delete ui;
}

void StudentProfile::onChangePasswordClicked()
{
    ChangePassword passwordDialog(this);
    passwordDialog.exec();
}

void StudentProfile::onBackClicked()
{
    close();
}