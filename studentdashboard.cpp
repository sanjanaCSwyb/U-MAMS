#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "studentdashboard.h"
#include "ui_studentdashboard.h"
#include "myapplications.h"
#include "medicalapplication.h"
#include "notifications.h"
#include "studentprofile.h"
#include "changepassword.h"
#include <QMessageBox>

StudentDashboard::StudentDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::StudentDashboard)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Student)) return;

    // ==========================================
    // MY PROFILE
    // ==========================================

    connect(ui->btnProfile,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onProfileClicked);


    // ==========================================
    // MEDICAL APPLICATION
    // ==========================================

    connect(ui->btnMedicalApplication,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onMedicalApplicationClicked);


    // ==========================================
    // MY APPLICATIONS
    // ==========================================

    connect(ui->btnMyApplications,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onMyApplicationsClicked);


    // ==========================================
    // NOTIFICATIONS
    // ==========================================

    connect(ui->btnNotifications,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onNotificationsClicked);


    // ==========================================
    // CHANGE PASSWORD
    // ==========================================

    connect(ui->btnChangePassword,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onChangePasswordClicked);


    // ==========================================
    // LOGOUT
    // ==========================================

    connect(ui->btnLogout,
            &QPushButton::clicked,
            this,
            &StudentDashboard::onLogoutClicked);
    FigmaUi::install(this);
    refreshData();
}


// ==========================================
// DESTRUCTOR
// ==========================================

StudentDashboard::~StudentDashboard()
{
    delete ui;
}


// ==========================================
// MY PROFILE
// ==========================================

void StudentDashboard::onProfileClicked()
{
    if (!AppSession::isStudent()) return;
    StudentProfile profile(this);
    profile.exec();
    refreshData();
}


// ==========================================
// MEDICAL APPLICATION
// ==========================================

void StudentDashboard::onMedicalApplicationClicked()
{
    if (!AppSession::isStudent()) return;
    MedicalApplication application(this);

    application.exec();
    refreshData();
}


// ==========================================
// MY APPLICATIONS
// ==========================================

void StudentDashboard::onMyApplicationsClicked()
{
    if (!AppSession::isStudent()) return;
    MyApplications applications(this);

    applications.exec();
    refreshData();
}


// ==========================================
// NOTIFICATIONS
// ==========================================

void StudentDashboard::onNotificationsClicked()
{
    if (!AppSession::isStudent()) return;
    Notifications notifications(this);

    notifications.exec();
    refreshData();
}


// ==========================================
// CHANGE PASSWORD
// ==========================================

void StudentDashboard::onChangePasswordClicked()
{
    if (!AppSession::isStudent()) return;
    ChangePassword passwordDialog(this);
    passwordDialog.exec();
    refreshData();
}

// ==========================================
// LOGOUT
// ==========================================

void StudentDashboard::onLogoutClicked()
{
    if (!AppSession::isStudent()) return;
    QMessageBox::information(
        this,
        "Logout",
        "You have been logged out."
        );

    AppSession::logout();
    close();
}
// ==========================================
// My Aplications
// ==========================================



void StudentDashboard::refreshData()
{
    if (!AppSession::isStudent()) return;
    QStringList errors;
    AppSession::Identity identity=AppSession::identity();
    AppSession::Identity current;
    if (Database::profile(current)) identity=current;
    else errors << Database::error();
    const QString welcome=identity.fullName.isEmpty() ? QString("Welcome") : QString("Welcome, %1").arg(identity.fullName);
    ui->lblWelcome->setText(ui->lblWelcome->fontMetrics().elidedText(welcome,Qt::ElideRight,ui->lblWelcome->width()));
    ui->lblWelcome->setToolTip(welcome);
    ui->lblStudentID->setText(QString("Student ID: %1").arg(identity.studentId.isEmpty() ? AppSession::principal() : identity.studentId));
    ui->lblDepartment->setText(identity.department);
    ReportSummary summary;
    if (Database::summary(summary)) {
        ui->lblPendingCount->setText(QString::number(summary.pending));
        ui->lblApprovedCount->setText(QString::number(summary.approved));
        ui->lblDeclinedCount->setText(QString::number(summary.declined));
    } else {
        ui->lblPendingCount->setText(QStringLiteral("—"));
        ui->lblApprovedCount->setText(QStringLiteral("—"));
        ui->lblDeclinedCount->setText(QStringLiteral("—"));
        errors << Database::error();
    }
    QList<ApplicationRecord> applications;
    if (!Database::applications(applications,AppSession::principal())) {
        ui->lblLatestApplication->setText("Unable to load your latest application.");
        errors << Database::error();
    } else if (applications.isEmpty()) {
        ui->lblLatestApplication->setText("No medical applications submitted yet.");
    } else {
        const ApplicationRecord *latest=&applications.first();
        for (const auto &application:applications)
            if (application.submittedAt>latest->submittedAt ||
                (application.submittedAt==latest->submittedAt && application.id>latest->id)) latest=&application;
        ui->lblLatestApplication->setText(QString("Latest: %1 - %2.").arg(Database::applicationCode(latest->id),latest->status));
    }
    errors.removeDuplicates();
    if (!errors.isEmpty()) QMessageBox::warning(this,"Unable to Refresh Dashboard",errors.join("\n\n"));
}
