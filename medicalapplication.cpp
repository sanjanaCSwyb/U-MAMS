#include "app_session.h"
#include "database.h"
#include "medicalapplication.h"
#include "figma_ui.h"
#include "ui_medicalapplication.h"

#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>

MedicalApplication::MedicalApplication(QWidget *parent)
    : QDialog(parent), ui(new Ui::MedicalApplication)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Student)) return;
    ui->txtStudentID->setReadOnly(true);
    ui->txtStudentName->setReadOnly(true);
    ui->txtMedicalID->setReadOnly(true);
    ui->txtMedicalID->clear();
    ui->txtMedicalID->setPlaceholderText(tr("Generated when submitted"));
    ui->txtMedicalID->setToolTip(tr("Assigned after submission using your Student ID, submission date and daily submission number."));
    ui->txtStudentID->setText(AppSession::principal());
    ui->txtStudentName->setText(AppSession::identity().fullName);
    AppSession::Identity identity;
    if (Database::profile(identity)) {
        ui->txtStudentID->setText(identity.studentId);
        ui->txtStudentName->setText(identity.fullName);
    } else {
        QMessageBox::warning(this, "Student Profile", Database::error());
    }
    connect(ui->btnBack, &QPushButton::clicked, this, &QDialog::reject);
    ui->dateFrom->setCalendarPopup(true);
    ui->dateTo->setCalendarPopup(true);
    ui->dateFrom->setDisplayFormat("dd/MM/yyyy");
    ui->dateTo->setDisplayFormat("dd/MM/yyyy");
    ui->dateFrom->setDate(QDate::currentDate());
    ui->dateTo->setDate(QDate::currentDate());
    ui->tblCourses->setColumnCount(2);
    ui->tblCourses->setHorizontalHeaderLabels({"Course Name", "Course Code"});
    ui->tblCourses->setRowCount(0);
    ui->tblCourses->verticalHeader()->hide();
    ui->tblCourses->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblCourses->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tblCourses->horizontalHeader()->setStretchLastSection(true);
    ui->tblCourses->setColumnWidth(0,420);
    ui->tblCourses->setColumnWidth(1,448);
    ui->tblCourses->verticalHeader()->setDefaultSectionSize(42);
    connect(ui->btnAddCourse,&QPushButton::clicked,this,&MedicalApplication::onAddCourseClicked);
    connect(ui->btnSubmit,&QPushButton::clicked,this,&MedicalApplication::onSubmitApplicationClicked);
    FigmaUi::install(this);
}

MedicalApplication::~MedicalApplication() { delete ui; }

void MedicalApplication::onAddCourseClicked()
{
    if (!AppSession::isStudent()) return;
    const QString name=ui->txtCourseName->text().trimmed();
    const QString code=ui->txtCourseCode->text().trimmed();
    if (name.isEmpty() || code.isEmpty()) {
        QMessageBox::warning(this,"Missing Information","Please enter both Course Name and Course Code.");
        return;
    }
    for (int row=0; row<ui->tblCourses->rowCount(); ++row) {
        const auto *existingName=ui->tblCourses->item(row,0);
        const auto *existingCode=ui->tblCourses->item(row,1);
        if (existingName && existingCode && existingName->text().compare(name,Qt::CaseInsensitive)==0 &&
            existingCode->text().compare(code,Qt::CaseInsensitive)==0) {
            QMessageBox::warning(this,"Duplicate Course","This course has already been added.");
            return;
        }
    }
    const int row=ui->tblCourses->rowCount();
    ui->tblCourses->insertRow(row);
    ui->tblCourses->setItem(row,0,new QTableWidgetItem(name));
    ui->tblCourses->setItem(row,1,new QTableWidgetItem(code));
    ui->txtCourseName->clear();
    ui->txtCourseCode->clear();
    ui->txtCourseName->setFocus();
}

void MedicalApplication::onSubmitApplicationClicked()
{
    if (!AppSession::isStudent()) return;
    const QString studentId=AppSession::principal();
    const QString reason=ui->txtReasonOfAbsence->toPlainText().trimmed();
    if (studentId.isEmpty() || ui->txtStudentID->text().trimmed()!=studentId) {
        QMessageBox::warning(this,"Student Account","Your student account could not be verified. Please sign in again.");
        return;
    }
    if (reason.isEmpty()) {
        QMessageBox::warning(this,"Missing Information","Please enter a reason for absence.");
        return;
    }
    const QDate from=ui->dateFrom->date(), to=ui->dateTo->date();
    if (!from.isValid() || !to.isValid() || from>to) {
        QMessageBox::warning(this,"Invalid Medical Period","Please select valid dates. The From date cannot be later than the To date.");
        return;
    }
    if (ui->tblCourses->rowCount()==0) {
        QMessageBox::warning(this,"Missing Course","Please add at least one missed course.");
        return;
    }
    QList<CourseRecord> courses;
    for (int row=0; row<ui->tblCourses->rowCount(); ++row) {
        const auto *name=ui->tblCourses->item(row,0);
        const auto *code=ui->tblCourses->item(row,1);
        if (!name || !code || name->text().trimmed().isEmpty() || code->text().trimmed().isEmpty()) {
            QMessageBox::warning(this,"Incomplete Course","Please make sure every course has both a Course Name and Course Code.");
            return;
        }
        courses.append({name->text().trimmed(),code->text().trimmed()});
    }
    AppSession::Identity student;
    if (!Database::profile(student)) {
        QMessageBox::warning(this,"Student Profile",Database::error());
        return;
    }
    if (student.role!=AppSession::Role::Student || student.studentId!=studentId) {
        QMessageBox::warning(this,"Student Account","Your student account could not be verified. Please sign in again.");
        return;
    }
    ui->txtStudentName->setText(student.fullName);
    int applicationId=0;
    QString medicalCode;
    ui->btnSubmit->setEnabled(false);
    const bool submitted=Database::submitApplication(studentId,reason,from,to,courses,applicationId,&medicalCode);
    ui->btnSubmit->setEnabled(true);
    if (!submitted) {
        QMessageBox::critical(this,"Application Not Submitted",Database::error());
        return;
    }
    // These identifiers refer to the committed record, never to a draft form.
    const QString applicationCode=Database::applicationCode(applicationId);
    // Leave input intact on failure; reset only after the repository commits.
    ui->txtMedicalID->clear();
    ui->txtMedicalID->setPlaceholderText(tr("Generated when submitted"));
    ui->txtReasonOfAbsence->clear();
    ui->txtCourseName->clear();
    ui->txtCourseCode->clear();
    ui->tblCourses->setRowCount(0);
    ui->dateFrom->setDate(QDate::currentDate());
    ui->dateTo->setDate(QDate::currentDate());
    QMessageBox::information(this,"Application Submitted",
        tr("Medical application submitted successfully.\n\nApplication ID: %1\nMedical ID: %2")
            .arg(applicationCode,medicalCode));
}
