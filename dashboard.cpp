#include "dashboard.h"
#include "ui_dashboard.h"

#include "medicalapplication.h"
#include "dialog.h"
#include "approvedecline.h"
#include "notifications.h"
#include "reports.h"
#include "changepassword.h"

#include <QMessageBox>
#include <QPushButton>


Dashboard::Dashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dashboard)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - Admin Dashboard");


    // =====================================================
    // VIEW APPLICATIONS
    // =====================================================

    connect(
        ui->btnViewApplications,
        &QPushButton::clicked,
        this,
        &Dashboard::onViewApplicationsClicked
        );


    // =====================================================
    // APPROVE / DECLINE
    // =====================================================

    connect(
        ui->btnApproveDecline,
        &QPushButton::clicked,
        this,
        &Dashboard::onApproveDeclineClicked
        );


    // =====================================================
    // NOTIFICATIONS
    // =====================================================

    connect(
        ui->btnNotifications,
        &QPushButton::clicked,
        this,
        &Dashboard::onNotificationsClicked
        );


    // =====================================================
    // REPORTS
    // =====================================================

    connect(
        ui->btnReports,
        &QPushButton::clicked,
        this,
        &Dashboard::onReportsClicked
        );


    // =====================================================
    // CHANGE PASSWORD
    // =====================================================

    connect(
        ui->btnChangePassword,
        &QPushButton::clicked,
        this,
        &Dashboard::onChangePasswordClicked
        );


    // =====================================================
    // LOGOUT
    // =====================================================

    connect(
        ui->btnLogout,
        &QPushButton::clicked,
        this,
        &Dashboard::onLogoutClicked
        );
}


// =========================================================
// DESTRUCTOR
// =========================================================

Dashboard::~Dashboard()
{
    delete ui;
}


// =========================================================
// MEDICAL APPLICATIONS
// =========================================================

void Dashboard::onMedicalApplicationsClicked()
{
    MedicalApplication application(this);
    application.exec();
}


// =========================================================
// VIEW APPLICATIONS
// =========================================================

void Dashboard::onViewApplicationsClicked()
{
    Dialog applications(this);
    applications.exec();
}


// =========================================================
// APPROVE / DECLINE
// =========================================================

void Dashboard::onApproveDeclineClicked()
{
    ApproveDecline approveDecline(this);
    approveDecline.exec();
}


// =========================================================
// NOTIFICATIONS
// =========================================================

void Dashboard::onNotificationsClicked()
{
    Notifications notifications(this);
    notifications.exec();
}


// =========================================================
// REPORTS
// =========================================================

void Dashboard::onReportsClicked()
{
    Reports reports(this);
    reports.exec();
}


// =========================================================
// CHANGE PASSWORD
// =========================================================

void Dashboard::onChangePasswordClicked()
{
    ChangePassword passwordDialog(this);

    passwordDialog.exec();
}


// =========================================================
// LOGOUT
// =========================================================

void Dashboard::onLogoutClicked()
{
    QMessageBox::information(
        this,
        "Logout",
        "You have been logged out successfully."
        );

    close();
}