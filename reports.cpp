#include "reports.h"
#include "app_session.h"
#include "database.h"
#include "ui_reports.h"
#include "figma_ui.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QSaveFile>
#include <QTableWidgetItem>

namespace {
QString csvField(QString value)
{
    if (!value.isEmpty() && QString("=+-@\t\r").contains(value.front())) value.prepend('\'');
    value.replace(QChar(0x22), QString(2, QChar(0x22)));
    return QChar(0x22) + value + QChar(0x22);
}
QString csvRow(const QStringList &values)
{
    QStringList escaped;
    for (const auto &value : values) escaped.append(csvField(value));
    return escaped.join(',') + "\r\n";
}
}

Reports::Reports(QWidget *parent) : QDialog(parent), ui(new Ui::Reports)
{
    ui->setupUi(this);
    if (!AppSession::protect(this,AppSession::Role::Admin)) return;
    ui->tblStatus->setColumnCount(3);
    ui->tblStatus->setHorizontalHeaderLabels({"Status","Count","Percentage"});
    ui->tblStatus->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblStatus->horizontalHeader()->setMinimumHeight(44);
    ui->tblStatus->verticalHeader()->hide();
    ui->tblStatus->verticalHeader()->setDefaultSectionSize(48);
    connect(ui->btnExportReport,&QPushButton::clicked,this,&Reports::onExportReportClicked);
    connect(ui->btnBack,&QPushButton::clicked,this,&Reports::onBackClicked);
    refreshData();
    FigmaUi::install(this);
}
Reports::~Reports() { delete ui; }

void Reports::refreshData()
{
    if (!AppSession::isAdmin()) return;
    ReportSummary summary;
    const bool loaded = Database::summary(summary);
    if (!loaded) summary = {};
    ui->lblTotal->setText(QString::number(summary.total));
    ui->lblPending->setText(QString::number(summary.pending));
    ui->lblApproved->setText(QString::number(summary.approved));
    ui->lblDeclined->setText(QString::number(summary.declined));
    ui->lblBarPending->setText(QString::number(summary.pending));
    ui->lblBarApproved->setText(QString::number(summary.approved));
    ui->lblBarDeclined->setText(QString::number(summary.declined));
    const int largest = qMax(summary.pending,qMax(summary.approved,summary.declined));
    const int maximum = qMax(1,(largest*3+1)/2);
    ui->barPending->setRange(0,maximum); ui->barPending->setValue(summary.pending);
    ui->barApproved->setRange(0,maximum); ui->barApproved->setValue(summary.approved);
    ui->barDeclined->setRange(0,maximum); ui->barDeclined->setValue(summary.declined);
    ui->tblStatus->setRowCount(loaded ? 3 : 0);
    const QStringList names = {"Pending","Approved","Declined"};
    const int counts[] = {summary.pending,summary.approved,summary.declined};
    if (loaded) for (int row=0; row<3; ++row) {
        const double percentage = summary.total > 0 ? 100.0*counts[row]/summary.total : 0.0;
        ui->tblStatus->setItem(row,0,new QTableWidgetItem(names[row]));
        ui->tblStatus->setItem(row,1,new QTableWidgetItem(QString::number(counts[row])));
        ui->tblStatus->setItem(row,2,new QTableWidgetItem(QString::number(percentage,'f',1)+'%'));
    }
    ui->reportsDescription->setText(loaded ? "Overview of submitted medical applications." : "Report data could not be loaded. Please try again.");
    ui->btnExportReport->setEnabled(loaded);
    if (!loaded) QMessageBox::critical(this,"Reports",Database::error());
}

void Reports::onExportReportClicked()
{
    if (!AppSession::isAdmin()) return;
    QList<ApplicationRecord> records;
    if (!Database::applications(records)) { QMessageBox::critical(this,"Export report",Database::error()); refreshData(); return; }
    // The list query supplies table fields; load each record's missed courses
    // before exporting so the CSV contains the actual submitted course data.
    for (auto &record : records) {
        ApplicationRecord complete;
        if (!Database::application(record.id,complete)) {
            QMessageBox::critical(this,"Export report",Database::error());
            refreshData();
            return;
        }
        record = complete;
    }
    QString path = QFileDialog::getSaveFileName(this,"Export report","U-MAMS-report.csv","CSV files (*.csv)");
    if (path.isEmpty() || !AppSession::isAdmin()) return;
    if (QFileInfo(path).suffix().isEmpty()) path += ".csv";
    ReportSummary summary;
    summary.total = records.size();
    for (const auto &record : records) {
        if (record.status == "Pending") ++summary.pending;
        else if (record.status == "Approved") ++summary.approved;
        else if (record.status == "Declined") ++summary.declined;
    }
    QString csv = csvRow({"U-MAMS Medical Absence Report"});
    csv += csvRow({"Generated",QDateTime::currentDateTime().toString(Qt::ISODate)});
    csv += csvRow({"Total",QString::number(summary.total)});
    csv += csvRow({"Pending",QString::number(summary.pending)});
    csv += csvRow({"Approved",QString::number(summary.approved)});
    csv += csvRow({"Declined",QString::number(summary.declined)});
    csv += "\r\n";
    csv += csvRow({"Application ID","Student ID","Student Name","Medical ID","From","To","Status","Reason","Missed Courses","Submitted","Reviewed"});
    for (const auto &record : records) {
        QStringList courses;
        for (const auto &course : record.courses) courses << course.name + " (" + course.code + ')';
        csv += csvRow({Database::applicationCode(record.id),record.studentId,record.studentName,record.medicalId,
                       record.fromDate.toString(Qt::ISODate),record.toDate.toString(Qt::ISODate),record.status,record.reason,
                       courses.join("; "),record.submittedAt.toString(Qt::ISODate),record.reviewedAt.toString(Qt::ISODate)});
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { QMessageBox::critical(this,"Export report",file.errorString()); return; }
    const QByteArray bytes = QByteArray::fromHex("EFBBBF") + csv.toUtf8();
    if (file.write(bytes) != bytes.size()) { const QString error=file.errorString(); file.cancelWriting(); QMessageBox::critical(this,"Export report",error); return; }
    if (!file.commit()) { QMessageBox::critical(this,"Export report",file.errorString()); return; }
    QMessageBox::information(this,"Export report","The report was saved to:\n" + path);
}
void Reports::onBackClicked() { if (AppSession::isAdmin()) close(); }
