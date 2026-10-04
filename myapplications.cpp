#include "myapplications.h"
#include "ui_myapplications.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>

MyApplications::MyApplications(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MyApplications)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - My Medical Applications");


    // ==========================================
    // SAMPLE APPLICATION DATA
    // ==========================================

    ui->tblApplications->setRowCount(3);
    ui->tblApplications->setColumnCount(5);

    ui->tblApplications->setHorizontalHeaderLabels(
        QStringList()
        << "Application ID"
        << "Medical ID"
        << "From"
        << "To"
        << "Status"
        );


    // Application 1
    ui->tblApplications->setItem(
        0, 0, new QTableWidgetItem("APP-001"));

    ui->tblApplications->setItem(
        0, 1,
        new QTableWidgetItem("249188-26-09-01-04"));

    ui->tblApplications->setItem(
        0, 2,
        new QTableWidgetItem("01/09/2026"));

    ui->tblApplications->setItem(
        0, 3,
        new QTableWidgetItem("04/09/2026"));

    ui->tblApplications->setItem(
        0, 4,
        new QTableWidgetItem("Pending"));


    // Application 2
    ui->tblApplications->setItem(
        1, 0, new QTableWidgetItem("APP-002"));

    ui->tblApplications->setItem(
        1, 1,
        new QTableWidgetItem("249188-26-08-05-02"));

    ui->tblApplications->setItem(
        1, 2,
        new QTableWidgetItem("05/08/2026"));

    ui->tblApplications->setItem(
        1, 3,
        new QTableWidgetItem("06/08/2026"));

    ui->tblApplications->setItem(
        1, 4,
        new QTableWidgetItem("Approved"));


    // Application 3
    ui->tblApplications->setItem(
        2, 0, new QTableWidgetItem("APP-003"));

    ui->tblApplications->setItem(
        2, 1,
        new QTableWidgetItem("249188-26-07-02-01"));

    ui->tblApplications->setItem(
        2, 2,
        new QTableWidgetItem("02/07/2026"));

    ui->tblApplications->setItem(
        2, 3,
        new QTableWidgetItem("04/07/2026"));

    ui->tblApplications->setItem(
        2, 4,
        new QTableWidgetItem("Declined"));


    // ==========================================
    // TABLE SETTINGS
    // ==========================================

    ui->tblApplications->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    ui->tblApplications->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    ui->tblApplications->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    ui->tblApplications->horizontalHeader()
        ->setStretchLastSection(true);

    ui->tblApplications->resizeColumnsToContents();


    // ==========================================
    // BUTTON CONNECTIONS
    // ==========================================

    connect(
        ui->btnViewDetails,
        &QPushButton::clicked,
        this,
        &MyApplications::onViewDetailsClicked
        );


    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &MyApplications::onBackClicked
        );
}


// ==========================================
// DESTRUCTOR
// ==========================================

MyApplications::~MyApplications()
{
    delete ui;
}


// ==========================================
// VIEW DETAILS
// ==========================================

void MyApplications::onViewDetailsClicked()
{
    int row = ui->tblApplications->currentRow();

    if (row < 0)
    {
        QMessageBox::warning(
            this,
            "No Application Selected",
            "Please select an application first."
            );

        return;
    }


    QString applicationID =
        ui->tblApplications->item(row, 0)->text();

    QString medicalID =
        ui->tblApplications->item(row, 1)->text();

    QString fromDate =
        ui->tblApplications->item(row, 2)->text();

    QString toDate =
        ui->tblApplications->item(row, 3)->text();

    QString status =
        ui->tblApplications->item(row, 4)->text();


    QString details =
        "Application ID: " + applicationID +
        "\n\n"
        "Medical ID: " + medicalID +
        "\n\n"
        "Medical Period: " +
        fromDate + " - " + toDate +
        "\n\n"
        "Status: " + status;


    QMessageBox::information(
        this,
        "Application Details",
        details
        );
}


// ==========================================
// BACK
// ==========================================

void MyApplications::onBackClicked()
{
    close();
}