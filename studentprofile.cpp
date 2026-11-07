#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "studentprofile.h"
#include "ui_studentprofile.h"
#include "changepassword.h"
#include <QMessageBox>
#include <QLabel>

StudentProfile::StudentProfile(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StudentProfile)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Student)) return;

    ui->txtStudentID->clear();
    ui->txtStudentName->clear();
    ui->txtEmail->clear();
    ui->txtDepartment->clear();

    AppSession::Identity user;
    if (Database::profile(user)) {
        ui->txtStudentID->setText(user.studentId);
        ui->txtStudentName->setText(user.fullName);
        ui->txtEmail->setText(user.email);
        ui->txtDepartment->setText(user.department);
    } else {
        auto *errorLabel = new QLabel(ui->figmaCanvas);
        errorLabel->setObjectName("profileLoadError");
        errorLabel->setGeometry(300, 450, 780, 36);
        errorLabel->setTextFormat(Qt::PlainText);
        errorLabel->setWordWrap(true);
        errorLabel->setText(tr("Unable to load your profile. %1").arg(Database::error()));
        errorLabel->setStyleSheet(
            "font-family:Inter;font-size:12px;color:#7A263A;background:transparent;border:none;");
    }

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
    FigmaUi::install(this);
}

StudentProfile::~StudentProfile()
{
    delete ui;
}

void StudentProfile::onChangePasswordClicked()
{
    if (!AppSession::isStudent()) return;
    ChangePassword passwordDialog(this);
    passwordDialog.exec();
}

void StudentProfile::onBackClicked()
{
    close();
}
