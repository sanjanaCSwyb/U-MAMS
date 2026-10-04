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
    StudentProfile profile(this);
    profile.exec();
}


// ==========================================
// MEDICAL APPLICATION
// ==========================================

void StudentDashboard::onMedicalApplicationClicked()
{
    MedicalApplication application(this);

    application.exec();
}


// ==========================================
// MY APPLICATIONS
// ==========================================

void StudentDashboard::onMyApplicationsClicked()
{
    MyApplications applications(this);

    applications.exec();
}


// ==========================================
// NOTIFICATIONS
// ==========================================

void StudentDashboard::onNotificationsClicked()
{
    Notifications notifications(this);

    notifications.exec();
}


// ==========================================
// CHANGE PASSWORD
// ==========================================

void StudentDashboard::onChangePasswordClicked()
{
    ChangePassword passwordDialog(this);
    passwordDialog.exec();
}

// ==========================================
// LOGOUT
// ==========================================

void StudentDashboard::onLogoutClicked()
{
    QMessageBox::information(
        this,
        "Logout",
        "You have been logged out."
        );

    close();
}
// ==========================================
// My Aplications
// ==========================================

