#include "approvedecline.h"
#include "ui_approvedecline.h"

#include <QMessageBox>
#include <QTableWidgetItem>


ApproveDecline::ApproveDecline(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApproveDecline)
{
    ui->setupUi(this);


    // ==========================================
    // SAMPLE APPLICATION DATA
    // ==========================================

    ui->lblApplicationID->setText("Application ID: APP-001");

    ui->lblStatus->setText("Status: PENDING");

    ui->lblStudentID->setText("Student ID: 249188");

    ui->lblStudentName->setText("Student Name: Kasun Kaushal");

    ui->lblMedicalID->setText(
        "Medical ID: 249188-26-09-01-04"
        );

    ui->lblMedicalPeriod->setText(
        "Medical Period: 01/09/2026 - 04/09/2026"
        );


    // ==========================================
    // MISSED COURSES
    // ==========================================

    ui->tblCourses->setRowCount(2);
    ui->tblCourses->setColumnCount(2);

    ui->tblCourses->setHorizontalHeaderLabels(
        QStringList() << "Course Name"
                      << "Course Code"
        );


    ui->tblCourses->setItem(
        0,
        0,
        new QTableWidgetItem("Physical Chemistry")
        );

    ui->tblCourses->setItem(
        0,
        1,
        new QTableWidgetItem("NANO 2132")
        );


    ui->tblCourses->setItem(
        1,
        0,
        new QTableWidgetItem("Nanomaterials")
        );

    ui->tblCourses->setItem(
        1,
        1,
        new QTableWidgetItem("NANO 2141")
        );


    // Make table read-only
    ui->tblCourses->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );


    // Resize columns
    ui->tblCourses->resizeColumnsToContents();


    // ==========================================
    // BUTTON CONNECTIONS
    // ==========================================

    connect(
        ui->btnApprove,
        &QPushButton::clicked,
        this,
        &ApproveDecline::onApproveClicked
        );


    connect(
        ui->btnDecline,
        &QPushButton::clicked,
        this,
        &ApproveDecline::onDeclineClicked
        );


    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &ApproveDecline::onBackClicked
        );
}


// ==========================================
// DESTRUCTOR
// ==========================================

ApproveDecline::~ApproveDecline()
{
    delete ui;
}


// ==========================================
// APPROVE
// ==========================================

void ApproveDecline::onApproveClicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(
        this,
        "Approve Application",
        "Are you sure you want to approve this medical absence application?",
        QMessageBox::Yes | QMessageBox::No
        );


    if (reply == QMessageBox::Yes)
    {
        ui->lblStatus->setText(
            "Status: APPROVED"
            );


        QMessageBox::information(
            this,
            "Application Approved",
            "The medical absence application has been approved."
            );
    }
}


// ==========================================
// DECLINE
// ==========================================

void ApproveDecline::onDeclineClicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(
        this,
        "Decline Application",
        "Are you sure you want to decline this medical absence application?",
        QMessageBox::Yes | QMessageBox::No
        );


    if (reply == QMessageBox::Yes)
    {
        ui->lblStatus->setText(
            "Status: DECLINED"
            );


        QMessageBox::information(
            this,
            "Application Declined",
            "The medical absence application has been declined."
            );
    }
}


// ==========================================
// BACK
// ==========================================

void ApproveDecline::onBackClicked()
{
    close();
}