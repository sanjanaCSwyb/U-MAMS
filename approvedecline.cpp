#include "approvedecline.h"
#include "app_session.h"
#include "database.h"
#include "figma_ui.h"
#include "ui_approvedecline.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>

ApproveDecline::ApproveDecline(QWidget *parent) : QDialog(parent), ui(new Ui::ApproveDecline)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Admin)) return;
    setWindowTitle("U-MAMS - Review Pending Applications");
    ui->tblCourses->setColumnCount(2);
    ui->tblCourses->setHorizontalHeaderLabels({"Course Name", "Course Code"});
    ui->tblCourses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblCourses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for (auto *label : {ui->lblApplicationID, ui->lblStudentID, ui->lblStudentName,
                       ui->lblMedicalID, ui->lblMedicalPeriod, ui->lblCoursesValue,
                       ui->lblReasonValue, ui->lblDecisionValue}) {
        label->setTextFormat(Qt::PlainText);
        label->setWordWrap(true);
    }
    connect(ui->btnApprove, &QPushButton::clicked, this, &ApproveDecline::onApproveClicked);
    connect(ui->btnDecline, &QPushButton::clicked, this, &ApproveDecline::onDeclineClicked);
    connect(ui->btnBack, &QPushButton::clicked, this, &ApproveDecline::onBackClicked);
    refreshData();
    FigmaUi::install(this);
}
ApproveDecline::~ApproveDecline() { delete ui; }

bool ApproveDecline::loadData(bool reportError)
{
    if (!AppSession::isAdmin()) return false;
    currentApplicationId = 0;
    for (auto *label : {ui->lblStudentID,ui->lblStudentName,ui->lblMedicalID,ui->lblMedicalPeriod,
                       ui->lblCoursesValue,ui->lblReasonValue}) label->clear();
    ui->tblCourses->setRowCount(0);
    ui->btnApprove->setEnabled(false);
    ui->btnDecline->setEnabled(false);
    ui->lblApplicationID->setText("No pending applications");
    ui->lblStatus->setText("NONE");
    ui->lblStatus->setStyleSheet("font-family:Inter; font-size:11px; color:#303333; background:#E2E8F0; border:none; border-radius:14px;");
    ui->lblDecisionValue->setText("There are no applications awaiting a decision.");
    ApplicationRecord record;
    if (!Database::nextPendingApplication(record)) {
        ui->lblApplicationID->setText("Unable to load applications");
        ui->lblStatus->setText("UNAVAILABLE");
        ui->lblDecisionValue->setText("Application data could not be loaded. Please try again.");
        if (reportError) QMessageBox::critical(this,"Pending applications",Database::error());
        return false;
    }
    if (record.id <= 0) return true;
    currentApplicationId = record.id;
    ui->lblApplicationID->setText(Database::applicationCode(record.id));
    ui->lblStatus->setText(record.status.toUpper());
    ui->lblStatus->setStyleSheet("font-family:Inter; font-size:11px; color:#34270A; background:#D6A21E; border:none; border-radius:14px;");
    ui->lblStudentID->setText(record.studentId + QString::fromUtf8(" — ") + record.studentName);
    ui->lblStudentName->setText(record.studentName);
    ui->lblMedicalID->setText(record.medicalId);
    ui->lblMedicalPeriod->setText(record.fromDate.toString("dd/MM/yyyy") + QString::fromUtf8(" — ") + record.toDate.toString("dd/MM/yyyy"));
    ui->lblReasonValue->setText(record.reason);
    ui->lblReasonValue->setToolTip(record.reason);
    ui->lblDecisionValue->setText("Review the full application before making a decision.");
    QStringList courses;
    for (const auto &course : record.courses) {
        courses << course.name + " (" + course.code + ')';
        const int row = ui->tblCourses->rowCount();
        ui->tblCourses->insertRow(row);
        ui->tblCourses->setItem(row,0,new QTableWidgetItem(course.name));
        ui->tblCourses->setItem(row,1,new QTableWidgetItem(course.code));
    }
    ui->lblCoursesValue->setText(courses.join(", "));
    ui->lblCoursesValue->setToolTip(courses.join("\n"));
    ui->btnApprove->setEnabled(record.status == "Pending");
    ui->btnDecline->setEnabled(record.status == "Pending");
    return true;
}
void ApproveDecline::refreshData() { if (AppSession::isAdmin()) loadData(true); }
void ApproveDecline::onApproveClicked() { if (AppSession::isAdmin()) reviewCurrent("Approved"); }
void ApproveDecline::onDeclineClicked() { if (AppSession::isAdmin()) reviewCurrent("Declined"); }
void ApproveDecline::reviewCurrent(const QString &status)
{
    if (!AppSession::isAdmin() || currentApplicationId <= 0) return;
    const int id = currentApplicationId;
    if (QMessageBox::question(this,"Review Application",QString("Mark %1 as %2?").arg(Database::applicationCode(id),status),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    if (!AppSession::isAdmin()) return;
    const bool saved = Database::reviewApplication(id,status);
    const QString error = saved ? QString() : Database::error();
    loadData(false);
    if (!saved) QMessageBox::critical(this,"Review failed",error);
}
void ApproveDecline::onBackClicked() { if (AppSession::isAdmin()) close(); }
