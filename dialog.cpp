#include "dialog.h"
#include "ui_dialog.h"
#include "applicationdetails.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - View Applications");


    // =====================================================
    // STATUS FILTER
    // =====================================================

    ui->cmbStatus->clear();

    ui->cmbStatus->addItem("All");
    ui->cmbStatus->addItem("Pending");
    ui->cmbStatus->addItem("Approved");
    ui->cmbStatus->addItem("Declined");


    // =====================================================
    // APPLICATION TABLE
    // =====================================================

    ui->tblApplications->setColumnCount(6);

    ui->tblApplications->setHorizontalHeaderLabels({
        "Application ID",
        "Student ID",
        "Student Name",
        "From",
        "To",
        "Status"
    });

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
        ->setSectionResizeMode(QHeaderView::Stretch);


    // =====================================================
    // DEMO APPLICATION DATA
    // =====================================================

    ui->tblApplications->setRowCount(3);


    // -----------------------------------------------------
    // APP-001
    // -----------------------------------------------------

    ui->tblApplications->setItem(
        0, 0,
        new QTableWidgetItem("APP-001")
        );

    ui->tblApplications->setItem(
        0, 1,
        new QTableWidgetItem("249188")
        );

    ui->tblApplications->setItem(
        0, 2,
        new QTableWidgetItem("Kasun")
        );

    ui->tblApplications->setItem(
        0, 3,
        new QTableWidgetItem("01/09/2026")
        );

    ui->tblApplications->setItem(
        0, 4,
        new QTableWidgetItem("04/09/2026")
        );

    ui->tblApplications->setItem(
        0, 5,
        new QTableWidgetItem("Pending")
        );


    // -----------------------------------------------------
    // APP-002
    // -----------------------------------------------------

    ui->tblApplications->setItem(
        1, 0,
        new QTableWidgetItem("APP-002")
        );

    ui->tblApplications->setItem(
        1, 1,
        new QTableWidgetItem("249172")
        );

    ui->tblApplications->setItem(
        1, 2,
        new QTableWidgetItem("Student 249172")
        );

    ui->tblApplications->setItem(
        1, 3,
        new QTableWidgetItem("05/08/2026")
        );

    ui->tblApplications->setItem(
        1, 4,
        new QTableWidgetItem("06/08/2026")
        );

    ui->tblApplications->setItem(
        1, 5,
        new QTableWidgetItem("Approved")
        );


    // -----------------------------------------------------
    // APP-003
    // -----------------------------------------------------

    ui->tblApplications->setItem(
        2, 0,
        new QTableWidgetItem("APP-003")
        );

    ui->tblApplications->setItem(
        2, 1,
        new QTableWidgetItem("249160")
        );

    ui->tblApplications->setItem(
        2, 2,
        new QTableWidgetItem("Student 249160")
        );

    ui->tblApplications->setItem(
        2, 3,
        new QTableWidgetItem("02/07/2026")
        );

    ui->tblApplications->setItem(
        2, 4,
        new QTableWidgetItem("04/07/2026")
        );

    ui->tblApplications->setItem(
        2, 5,
        new QTableWidgetItem("Declined")
        );


    // =====================================================
    // BUTTON CONNECTIONS
    // =====================================================

    connect(
        ui->btnSearch,
        &QPushButton::clicked,
        this,
        &Dialog::onSearchClicked
        );

    connect(
        ui->cmbStatus,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &Dialog::onStatusChanged
        );

    connect(
        ui->btnApprove,
        &QPushButton::clicked,
        this,
        &Dialog::onApproveClicked
        );

    connect(
        ui->btnDecline,
        &QPushButton::clicked,
        this,
        &Dialog::onDeclineClicked
        );

    connect(
        ui->btnViewDetails,
        &QPushButton::clicked,
        this,
        &Dialog::onViewDetailsClicked
        );

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &Dialog::onBackClicked
        );
}


// =========================================================
// DESTRUCTOR
// =========================================================

Dialog::~Dialog()
{
    delete ui;
}


// =========================================================
// SEARCH APPLICATIONS
// =========================================================

void Dialog::onSearchClicked()
{
    QString searchID =
        ui->txtSearchStudentID->text().trimmed();

    QString selectedStatus =
        ui->cmbStatus->currentText();


    for (int row = 0;
         row < ui->tblApplications->rowCount();
         ++row)
    {
        QString studentID =
            ui->tblApplications->item(row, 1)->text();

        QString status =
            ui->tblApplications->item(row, 5)->text();


        bool studentMatches = true;
        bool statusMatches = true;


        // Student ID search
        if (!searchID.isEmpty())
        {
            studentMatches =
                studentID.contains(
                    searchID,
                    Qt::CaseInsensitive
                    );
        }


        // Status filter
        if (selectedStatus != "All")
        {
            statusMatches =
                status.compare(
                    selectedStatus,
                    Qt::CaseInsensitive
                    ) == 0;
        }


        ui->tblApplications->setRowHidden(
            row,
            !(studentMatches && statusMatches)
            );
    }
}


// =========================================================
// STATUS FILTER
// =========================================================

void Dialog::onStatusChanged(int index)
{
    Q_UNUSED(index);

    onSearchClicked();
}


// =========================================================
// APPROVE APPLICATION
// =========================================================

void Dialog::onApproveClicked()
{
    int row =
        ui->tblApplications->currentRow();


    if (row < 0)
    {
        QMessageBox::warning(
            this,
            "Approve Application",
            "Please select an application first."
            );

        return;
    }


    QString applicationID =
        ui->tblApplications->item(row, 0)->text();

    QString currentStatus =
        ui->tblApplications->item(row, 5)->text();


    if (currentStatus == "Approved")
    {
        QMessageBox::information(
            this,
            "Approve Application",
            "This application is already approved."
            );

        return;
    }


    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Approve Application",
            "Are you sure you want to approve " +
                applicationID + "?",
            QMessageBox::Yes |
                QMessageBox::No
            );


    if (reply == QMessageBox::Yes)
    {
        ui->tblApplications->item(row, 5)
        ->setText("Approved");


        QMessageBox::information(
            this,
            "Application Approved",
            applicationID +
                " has been approved successfully."
            );


        onSearchClicked();
    }
}


// =========================================================
// DECLINE APPLICATION
// =========================================================

void Dialog::onDeclineClicked()
{
    int row =
        ui->tblApplications->currentRow();


    if (row < 0)
    {
        QMessageBox::warning(
            this,
            "Decline Application",
            "Please select an application first."
            );

        return;
    }


    QString applicationID =
        ui->tblApplications->item(row, 0)->text();

    QString currentStatus =
        ui->tblApplications->item(row, 5)->text();


    if (currentStatus == "Declined")
    {
        QMessageBox::information(
            this,
            "Decline Application",
            "This application is already declined."
            );

        return;
    }


    QMessageBox::StandardButton reply =
        QMessageBox::question(
            this,
            "Decline Application",
            "Are you sure you want to decline " +
                applicationID + "?",
            QMessageBox::Yes |
                QMessageBox::No
            );


    if (reply == QMessageBox::Yes)
    {
        ui->tblApplications->item(row, 5)
        ->setText("Declined");


        QMessageBox::information(
            this,
            "Application Declined",
            applicationID +
                " has been declined."
            );


        onSearchClicked();
    }
}


// =========================================================
// VIEW APPLICATION DETAILS
// =========================================================

void Dialog::onViewDetailsClicked()
{
    int row =
        ui->tblApplications->currentRow();


    if (row < 0)
    {
        QMessageBox::warning(
            this,
            "View Application",
            "Please select an application first."
            );

        return;
    }


    // =====================================================
    // GET SELECTED APPLICATION DATA
    // =====================================================

    QString applicationID =
        ui->tblApplications->item(row, 0)->text();

    QString studentID =
        ui->tblApplications->item(row, 1)->text();

    QString studentName =
        ui->tblApplications->item(row, 2)->text();

    QString fromDate =
        ui->tblApplications->item(row, 3)->text();

    QString toDate =
        ui->tblApplications->item(row, 4)->text();

    QString status =
        ui->tblApplications->item(row, 5)->text();


    // =====================================================
    // DEMO MEDICAL INFORMATION
    // =====================================================

    QString medicalID;
    QString reason;

    QList<QPair<QString, QString>> courses;


    if (applicationID == "APP-001")
    {
        medicalID = "249188-26-09-01-04";

        reason = "Medical absence";


        courses.append(
            qMakePair(
                QString("Physical Chemistry"),
                QString("NANO 2132")
                )
            );


        courses.append(
            qMakePair(
                QString("Nanomaterials"),
                QString("NANO 2141")
                )
            );
    }


    else if (applicationID == "APP-002")
    {
        medicalID = "249172-26-08-05-02";

        reason = "Medical absence";


        courses.append(
            qMakePair(
                QString("Materials Science"),
                QString("NANO 2141")
                )
            );


        courses.append(
            qMakePair(
                QString("Nanotechnology"),
                QString("NANO 2142")
                )
            );
    }


    else if (applicationID == "APP-003")
    {
        medicalID = "249160-26-07-02-01";

        reason = "Medical absence";


        courses.append(
            qMakePair(
                QString("Physical Chemistry"),
                QString("NANO 2132")
                )
            );
    }


    // =====================================================
    // OPEN APPLICATION DETAILS
    // =====================================================

    ApplicationDetails details(this);


    // =====================================================
    // RECEIVE STATUS CHANGES
    // =====================================================

    connect(
        &details,
        &ApplicationDetails::statusChanged,
        this,
        [this](
            const QString &applicationID,
            const QString &newStatus
            )
        {
            for (int row = 0;
                 row < ui->tblApplications->rowCount();
                 ++row)
            {
                QTableWidgetItem *item =
                    ui->tblApplications->item(row, 0);


                if (item &&
                    item->text() == applicationID)
                {
                    ui->tblApplications
                        ->item(row, 5)
                        ->setText(newStatus);

                    break;
                }
            }
        }
        );


    // =====================================================
    // SEND DATA TO APPLICATION DETAILS
    // =====================================================

    details.setApplicationData(
        applicationID,
        studentID,
        studentName,
        medicalID,
        reason,
        fromDate,
        toDate,
        status,
        courses
        );


    // Show details window
    details.exec();
}


// =========================================================
// BACK
// =========================================================

void Dialog::onBackClicked()
{
    close();
}