#include "reports.h"
#include "ui_reports.h"

#include <QMessageBox>
#include <QTableWidgetItem>

Reports::Reports(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Reports)
{
    ui->setupUi(this);

    // ==========================================
    // TEST / DEMO REPORT DATA
    // ==========================================

    ui->lblTotal->setText("12");
    ui->lblPending->setText("5");
    ui->lblApproved->setText("6");
    ui->lblDeclined->setText("1");


    // ==========================================
    // STATUS TABLE
    // ==========================================

    ui->tblStatus->setRowCount(3);
    ui->tblStatus->setColumnCount(3);

    ui->tblStatus->setHorizontalHeaderLabels(
        QStringList() << "Status"
                      << "Count"
                      << "Percentage"
        );


    // Pending
    ui->tblStatus->setItem(
        0, 0,
        new QTableWidgetItem("Pending")
        );

    ui->tblStatus->setItem(
        0, 1,
        new QTableWidgetItem("5")
        );

    ui->tblStatus->setItem(
        0, 2,
        new QTableWidgetItem("41.7%")
        );


    // Approved
    ui->tblStatus->setItem(
        1, 0,
        new QTableWidgetItem("Approved")
        );

    ui->tblStatus->setItem(
        1, 1,
        new QTableWidgetItem("6")
        );

    ui->tblStatus->setItem(
        1, 2,
        new QTableWidgetItem("50.0%")
        );


    // Declined
    ui->tblStatus->setItem(
        2, 0,
        new QTableWidgetItem("Declined")
        );

    ui->tblStatus->setItem(
        2, 1,
        new QTableWidgetItem("1")
        );

    ui->tblStatus->setItem(
        2, 2,
        new QTableWidgetItem("8.3%")
        );


    // Adjust table
    ui->tblStatus->resizeColumnsToContents();
    ui->tblStatus->horizontalHeader()->setStretchLastSection(true);


    // ==========================================
    // BUTTON CONNECTIONS
    // ==========================================

    connect(
        ui->btnExportReport,
        &QPushButton::clicked,
        this,
        &Reports::onExportReportClicked
        );


    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &Reports::onBackClicked
        );
}


// ==========================================
// DESTRUCTOR
// ==========================================

Reports::~Reports()
{
    delete ui;
}


// ==========================================
// EXPORT REPORT
// ==========================================

void Reports::onExportReportClicked()
{
    QMessageBox::information(
        this,
        "Export Report",
        "Report export functionality will be connected later.\n\n"
        "Current report summary:\n\n"
        "Total Applications: 12\n"
        "Pending: 5\n"
        "Approved: 6\n"
        "Declined: 1"
        );
}


// ==========================================
// BACK TO DASHBOARD
// ==========================================

void Reports::onBackClicked()
{
    close();
}