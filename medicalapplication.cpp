#include "medicalapplication.h"
#include "ui_medicalapplication.h"

#include <QMessageBox>
#include <QTableWidgetItem>


MedicalApplication::MedicalApplication(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MedicalApplication)
{
    ui->setupUi(this);

    // Set course table
    ui->tblCourses->setColumnCount(2);
    ui->tblCourses->setHorizontalHeaderLabels(
        QStringList() << "Course Name" << "Course Code"
        );

    ui->tblCourses->horizontalHeader()->setStretchLastSection(true);
    ui->tblCourses->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch
        );

    // Initially no courses
    ui->tblCourses->setRowCount(0);

    // Connect Add Course button
    connect(ui->btnAddCourse, &QPushButton::clicked,
            this, &MedicalApplication::onAddCourseClicked);

    // Connect Submit button
    connect(ui->btnSubmit, &QPushButton::clicked,
            this, &MedicalApplication::onSubmitApplicationClicked);
}


MedicalApplication::~MedicalApplication()
{
    delete ui;
}


// ==========================================
// ADD COURSE
// ==========================================

void MedicalApplication::onAddCourseClicked()
{
    // Get course information
    QString courseName = ui->txtCourseName->text().trimmed();
    QString courseCode = ui->txtCourseCode->text().trimmed();

    // Check whether both fields are filled
    if (courseName.isEmpty() || courseCode.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Information",
            "Please enter both Course Name and Course Code."
            );

        return;
    }


    // Check whether the same course already exists
    for (int row = 0; row < ui->tblCourses->rowCount(); ++row)
    {
        QString existingName =
            ui->tblCourses->item(row, 0)->text();

        QString existingCode =
            ui->tblCourses->item(row, 1)->text();

        if (existingName.compare(courseName, Qt::CaseInsensitive) == 0 &&
            existingCode.compare(courseCode, Qt::CaseInsensitive) == 0)
        {
            QMessageBox::warning(
                this,
                "Duplicate Course",
                "This course has already been added."
                );

            return;
        }
    }


    // Add a new row to the table
    int newRow = ui->tblCourses->rowCount();

    ui->tblCourses->insertRow(newRow);

    // Add Course Name
    ui->tblCourses->setItem(
        newRow,
        0,
        new QTableWidgetItem(courseName)
        );

    // Add Course Code
    ui->tblCourses->setItem(
        newRow,
        1,
        new QTableWidgetItem(courseCode)
        );


    // Clear input fields
    ui->txtCourseName->clear();
    ui->txtCourseCode->clear();

    // Put cursor back into Course Name
    ui->txtCourseName->setFocus();
}


// ==========================================
// SUBMIT APPLICATION
// ==========================================

void MedicalApplication::onSubmitApplicationClicked()
{
    // ==========================================
    // GET STUDENT INFORMATION
    // ==========================================

    QString studentID =
        ui->txtStudentID->text().trimmed();

    QString studentName =
        ui->txtStudentName->text().trimmed();

    QString medicalID =
        ui->txtMedicalID->text().trimmed();

    QString reason =
        ui->txtReasonOfAbsence->toPlainText().trimmed();


    // ==========================================
    // GET MEDICAL PERIOD
    // ==========================================

    QDate fromDate =
        ui->dateFrom->date();

    QDate toDate =
        ui->dateTo->date();


    // ==========================================
    // CHECK REQUIRED INFORMATION
    // ==========================================

    if (studentID.isEmpty() ||
        studentName.isEmpty() ||
        medicalID.isEmpty() ||
        reason.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Information",
            "Please fill in all required student and medical information."
            );

        return;
    }


    // ==========================================
    // CHECK MEDICAL PERIOD
    // ==========================================

    if (fromDate > toDate)
    {
        QMessageBox::warning(
            this,
            "Invalid Medical Period",
            "The Medical Period From date cannot be later than the To date."
            );

        return;
    }


    // ==========================================
    // CHECK AT LEAST ONE COURSE
    // ==========================================

    if (ui->tblCourses->rowCount() == 0)
    {
        QMessageBox::warning(
            this,
            "Missing Course",
            "Please add at least one missed course."
            );

        return;
    }


    // ==========================================
    // CHECK THAT ALL COURSE DATA IS COMPLETE
    // ==========================================

    for (int row = 0;
         row < ui->tblCourses->rowCount();
         ++row)
    {
        QTableWidgetItem *nameItem =
            ui->tblCourses->item(row, 0);

        QTableWidgetItem *codeItem =
            ui->tblCourses->item(row, 1);


        if (nameItem == nullptr ||
            codeItem == nullptr ||
            nameItem->text().trimmed().isEmpty() ||
            codeItem->text().trimmed().isEmpty())
        {
            QMessageBox::warning(
                this,
                "Incomplete Course",
                "Please make sure every course has both a Course Name and Course Code."
                );

            return;
        }
    }


    // ==========================================
    // COLLECT ALL COURSE DETAILS
    // ==========================================

    QString courses;

    for (int row = 0;
         row < ui->tblCourses->rowCount();
         ++row)
    {
        QString courseName =
            ui->tblCourses->item(row, 0)->text();

        QString courseCode =
            ui->tblCourses->item(row, 1)->text();


        courses += courseName +
                   " (" +
                   courseCode +
                   ")";


        // Add new line between courses
        if (row < ui->tblCourses->rowCount() - 1)
        {
            courses += "\n";
        }
    }


    // ==========================================
    // TEMPORARY CONFIRMATION
    // ==========================================
    //
    // Later we will use these collected details
    // to save the application and send emails.
    //

    QString applicationDetails;

    applicationDetails +=
        "Student ID: " + studentID + "\n";

    applicationDetails +=
        "Student Name: " + studentName + "\n";

    applicationDetails +=
        "Medical ID: " + medicalID + "\n";

    applicationDetails +=
        "Medical Period: " +
        fromDate.toString("dd/MM/yyyy") +
        " - " +
        toDate.toString("dd/MM/yyyy") +
        "\n";

    applicationDetails +=
        "Reason: " + reason + "\n\n";

    applicationDetails +=
        "Missed Courses:\n" +
        courses;


    QMessageBox::information(
        this,
        "Application Submitted",
        "Medical application submitted successfully!\n\n" +
            applicationDetails
        );
}