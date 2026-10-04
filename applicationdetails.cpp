#include "applicationdetails.h"
#include "ui_applicationdetails.h"

#include <QPushButton>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>


ApplicationDetails::ApplicationDetails(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ApplicationDetails)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - Application Details");


    // ==========================================
    // INFORMATION LABELS
    // ==========================================

    ui->lblApplicationID->setMinimumWidth(650);
    ui->lblStatus->setMinimumWidth(650);
    ui->lblStudentID->setMinimumWidth(650);
    ui->lblStudentName->setMinimumWidth(650);
    ui->lblMedicalID->setMinimumWidth(650);
    ui->lblReason->setMinimumWidth(650);
    ui->lblMedicalPeriod->setMinimumWidth(650);

    ui->lblApplicationID->setWordWrap(true);
    ui->lblStatus->setWordWrap(true);
    ui->lblStudentID->setWordWrap(true);
    ui->lblStudentName->setWordWrap(true);
    ui->lblMedicalID->setWordWrap(true);
    ui->lblReason->setWordWrap(true);
    ui->lblMedicalPeriod->setWordWrap(true);

    setMinimumWidth(850);


    // ==========================================
    // COURSES TABLE
    // ==========================================

    ui->tblCourses->setColumnCount(2);

    ui->tblCourses->setHorizontalHeaderLabels({
        "Course Name",
        "Course Code"
    });

    ui->tblCourses->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    ui->tblCourses->horizontalHeader()
        ->setStretchLastSection(true);

    ui->tblCourses->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);


    // ==========================================
    // BACK BUTTON
    // ==========================================

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &ApplicationDetails::onBackClicked
        );


    // ==========================================
    // APPROVE BUTTON
    // ==========================================

    connect(
        ui->btnApprove,
        &QPushButton::clicked,
        this,
        &ApplicationDetails::onApproveClicked
        );
}


// ==========================================
// DESTRUCTOR
// ==========================================

ApplicationDetails::~ApplicationDetails()
{
    delete ui;
}


// ==========================================
// SET APPLICATION DATA
// ==========================================

void ApplicationDetails::setApplicationData(
    const QString &applicationID,
    const QString &studentID,
    const QString &studentName,
    const QString &medicalID,
    const QString &reason,
    const QString &fromDate,
    const QString &toDate,
    const QString &status,
    const QList<QPair<QString, QString>> &courses
    )
{
    // Save application ID
    currentApplicationID = applicationID;


    // ------------------------------------------
    // APPLICATION ID
    // ------------------------------------------

    ui->lblApplicationID->setText(
        "Application ID: " + applicationID
        );


    // ------------------------------------------
    // STATUS
    // ------------------------------------------

    ui->lblStatus->setText(
        "Status: " + status
        );


    // ------------------------------------------
    // STUDENT ID
    // ------------------------------------------

    ui->lblStudentID->setText(
        "Student ID: " + studentID
        );


    // ------------------------------------------
    // STUDENT NAME
    // ------------------------------------------

    ui->lblStudentName->setText(
        "Student Name: " + studentName
        );


    // ------------------------------------------
    // MEDICAL ID
    // ------------------------------------------

    ui->lblMedicalID->setText(
        "Medical ID: " + medicalID
        );


    // ------------------------------------------
    // REASON
    // ------------------------------------------

    ui->lblReason->setText(
        "Reason for Absence: " + reason
        );


    // ------------------------------------------
    // MEDICAL PERIOD
    // ------------------------------------------

    ui->lblMedicalPeriod->setText(
        "Medical Period: " +
        fromDate +
        " - " +
        toDate
        );


    // ------------------------------------------
    // MISSED COURSES
    // ------------------------------------------

    ui->tblCourses->setRowCount(0);

    for (const auto &course : courses)
    {
        int row = ui->tblCourses->rowCount();

        ui->tblCourses->insertRow(row);

        ui->tblCourses->setItem(
            row,
            0,
            new QTableWidgetItem(course.first)
            );

        ui->tblCourses->setItem(
            row,
            1,
            new QTableWidgetItem(course.second)
            );
    }

    ui->tblCourses->resizeColumnsToContents();
}


// ==========================================
// BACK
// ==========================================

void ApplicationDetails::onBackClicked()
{
    close();
}


// ==========================================
// APPROVE
// ==========================================

void ApplicationDetails::onApproveClicked()
{
    QString currentStatus =
        ui->lblStatus->text();


    // Prevent approving an already approved application
    if (currentStatus.contains("Approved",
                               Qt::CaseInsensitive))
    {
        QMessageBox::information(
            this,
            "Application",
            "This application is already approved."
            );

        return;
    }


    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Approve Application",
            "Are you sure you want to approve this application?",
            QMessageBox::Yes |
                QMessageBox::No
            );


    if (reply == QMessageBox::Yes)
    {
        // Update status in details window
        ui->lblStatus->setText(
            "Status: Approved"
            );


        // Tell the View Applications window
        emit statusChanged(
            currentApplicationID,
            "Approved"
            );


        QMessageBox::information(
            this,
            "Application Approved",
            "The application has been approved successfully."
            );
    }
}