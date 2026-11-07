#include "dashboard.h"
#include "app_session.h"
#include "database.h"
#include "ui_dashboard.h"
#include "figma_ui.h"
#include "dialog.h"
#include "approvedecline.h"
#include "notifications.h"
#include "reports.h"
#include "monthlyapprovedmedicals.h"
#include "changepassword.h"
#include <QMessageBox>
#include <QPushButton>
#include <algorithm>

Dashboard::Dashboard(QWidget *parent) : QDialog(parent), ui(new Ui::Dashboard)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Admin)) return;
    setWindowTitle("U-MAMS - Admin Dashboard");
    connect(ui->btnQuickReview,&QPushButton::clicked,this,&Dashboard::onApproveDeclineClicked);
    connect(ui->btnQuickReports,&QPushButton::clicked,this,&Dashboard::onReportsClicked);
    connect(ui->btnQuickMonthlyMedicals,&QPushButton::clicked,this,&Dashboard::onMonthlyMedicalsClicked);
    connect(ui->btnQuickNotifications,&QPushButton::clicked,this,&Dashboard::onNotificationsClicked);
    connect(ui->btnViewApplications,&QPushButton::clicked,this,&Dashboard::onViewApplicationsClicked);
    connect(ui->btnApproveDecline,&QPushButton::clicked,this,&Dashboard::onApproveDeclineClicked);
    connect(ui->btnNotifications,&QPushButton::clicked,this,&Dashboard::onNotificationsClicked);
    connect(ui->btnReports,&QPushButton::clicked,this,&Dashboard::onReportsClicked);
    connect(ui->btnChangePassword,&QPushButton::clicked,this,&Dashboard::onChangePasswordClicked);
    connect(ui->btnLogout,&QPushButton::clicked,this,&Dashboard::onLogoutClicked);
    ui->label->setTextFormat(Qt::PlainText);
    ui->recentSubmitted->setTextFormat(Qt::PlainText);
    ui->recentApproved->setTextFormat(Qt::PlainText);
    refreshData();
    FigmaUi::install(this);
}
Dashboard::~Dashboard() { delete ui; }

void Dashboard::refreshData()
{
    if (!AppSession::isAdmin()) return;
    for (auto *label : {ui->summaryMedicalValue,ui->summaryPendingValue,ui->summaryApprovedValue,ui->summaryDeclinedValue}) label->setText("0");
    ui->recentSubmitted->clear();
    ui->recentApproved->clear();
    const auto identity = AppSession::identity();
    const QString name = !identity.fullName.isEmpty() ? identity.fullName : !identity.username.isEmpty() ? identity.username : "Admin";
    ui->label->setText("Welcome, " + name);
    ReportSummary summary;
    QList<ApplicationRecord> applications;
    if (!Database::summary(summary) || !Database::applications(applications)) {
        ui->dashboardDescription->setText("Application data could not be loaded. Please try again.");
        QMessageBox::critical(this,"Dashboard",Database::error());
        return;
    }
    ui->dashboardDescription->setText("Manage student medical absence applications from one place.");
    ui->summaryMedicalValue->setText(QString::number(summary.total));
    ui->summaryPendingValue->setText(QString::number(summary.pending));
    ui->summaryApprovedValue->setText(QString::number(summary.approved));
    ui->summaryDeclinedValue->setText(QString::number(summary.declined));
    struct Activity { QDateTime when; int id; QString text; };
    QList<Activity> activity;
    for (const auto &record : applications) {
        const QString code = Database::applicationCode(record.id);
        activity.append({record.submittedAt,record.id,QString("%1 submitted - Student %2").arg(code,record.studentId)});
        if (record.reviewedAt.isValid() && record.status != "Pending")
            activity.append({record.reviewedAt,record.id,QString("%1 %2 - Student %3").arg(code,record.status.toLower(),record.studentId)});
    }
    std::sort(activity.begin(),activity.end(),[](const Activity &a,const Activity &b) {
        return a.when == b.when ? a.id > b.id : a.when > b.when;
    });
    ui->recentSubmitted->setText(activity.isEmpty() ? "No applications submitted yet." : activity[0].text);
    ui->recentApproved->setText(activity.size() > 1 ? activity[1].text : QString());
}

void Dashboard::onMedicalApplicationsClicked()
{
    if (!AppSession::isAdmin()) return;
    // Retained for existing callers: administrators inspect submitted requests.
    onViewApplicationsClicked();
}
void Dashboard::onViewApplicationsClicked()
{
    if (!AppSession::isAdmin()) return;
    Dialog page(this); page.exec(); refreshData();
}
void Dashboard::onApproveDeclineClicked()
{
    if (!AppSession::isAdmin()) return;
    ApproveDecline page(this); page.exec(); refreshData();
}
void Dashboard::onNotificationsClicked()
{
    if (!AppSession::isAdmin()) return;
    Notifications page(this); page.exec(); refreshData();
}
void Dashboard::onReportsClicked()
{
    if (!AppSession::isAdmin()) return;
    Reports page(this); page.exec(); refreshData();
}
void Dashboard::onMonthlyMedicalsClicked()
{
    if (!AppSession::isAdmin()) return;
    MonthlyApprovedMedicals page(this); page.exec(); refreshData();
}
void Dashboard::onChangePasswordClicked()
{
    if (!AppSession::isAdmin()) return;
    ChangePassword page(this); page.exec(); refreshData();
}
void Dashboard::onLogoutClicked()
{
    if (!AppSession::isAdmin()) return;
    AppSession::logout(); close();
}
