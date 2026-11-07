#include "app_session.h"
#include "applicationdetails.h"
#include "database.h"
#include "figma_ui.h"
#include "ui_applicationdetails.h"
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QDate>


#include <QPushButton>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>


namespace {
class CourseLineDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        QStyleOptionViewItem clean(option);
        clean.state &= ~QStyle::State_HasFocus;
        QStyledItemDelegate::paint(painter, clean, index);
    }
protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option,index);
        option->text.clear();
    }
};
}

ApplicationDetails::ApplicationDetails(QWidget *parent) : QDialog(parent), ui(new Ui::ApplicationDetails)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Admin)) return;
    setWindowTitle("U-MAMS - Application Details");
    for (auto *label : {ui->lblApplicationID,ui->lblStatus,ui->lblStudentID,ui->lblStudentName,
                       ui->lblMedicalID,ui->lblReason,ui->lblMedicalPeriod}) {
        label->setWordWrap(true);
        label->setTextFormat(Qt::PlainText);
    }
    ui->tblCourses->setColumnCount(2);
    ui->tblCourses->setHorizontalHeaderLabels({"Course Name","Course Code"});
    ui->tblCourses->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblCourses->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tblCourses->horizontalHeader()->hide();
    ui->tblCourses->verticalHeader()->hide();
    ui->tblCourses->verticalHeader()->setDefaultSectionSize(40);
    ui->tblCourses->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tblCourses->setItemDelegate(new CourseLineDelegate(ui->tblCourses));
    connect(ui->btnBack,&QPushButton::clicked,this,&ApplicationDetails::onBackClicked);
    connect(ui->btnApprove,&QPushButton::clicked,this,&ApplicationDetails::onApproveClicked);
    clearData();
    FigmaUi::install(this);
}
ApplicationDetails::~ApplicationDetails() { delete ui; }

void ApplicationDetails::clearData()
{
    for (auto *label : {ui->lblApplicationID,ui->lblStatus,ui->lblStudentID,ui->lblStudentName,
                       ui->lblMedicalID,ui->lblReason,ui->lblMedicalPeriod}) label->clear();
    ui->lblApplicationID->setText("No application selected");
    ui->tblCourses->clearSpans();
    ui->tblCourses->setRowCount(0);
    ui->btnApprove->setEnabled(false);
}

bool ApplicationDetails::setApplicationId(int id)
{
    if (!AppSession::isAdmin()) return false;
    currentNumericId = id;
    currentApplicationID = id > 0 ? Database::applicationCode(id) : QString();
    return loadData(true);
}

void ApplicationDetails::setApplicationData(const QString &applicationID, const QString &studentID,
    const QString &studentName, const QString &medicalID, const QString &reason,
    const QString &fromDate, const QString &toDate, const QString &status,
    const QList<QPair<QString,QString>> &courses)
{
    Q_UNUSED(studentID); Q_UNUSED(studentName); Q_UNUSED(medicalID); Q_UNUSED(reason);
    Q_UNUSED(fromDate); Q_UNUSED(toDate); Q_UNUSED(status); Q_UNUSED(courses);
    if (!AppSession::isAdmin()) return;
    setApplicationId(Database::applicationId(applicationID));
}

bool ApplicationDetails::loadData(bool reportError)
{
    if (!AppSession::isAdmin()) return false;
    clearData();
    if (currentNumericId <= 0) return false;
    ApplicationRecord record;
    if (!Database::application(currentNumericId,record)) {
        ui->lblApplicationID->setText("Application unavailable");
        if (reportError) QMessageBox::critical(this,"Application",Database::error());
        return false;
    }
    currentApplicationID = Database::applicationCode(record.id);
    ui->lblApplicationID->setText("Application " + currentApplicationID);
    ui->lblStatus->setText(record.status.toUpper());
    const QString color = record.status == "Pending" ? "#34270A" : "white";
    const QString fill = record.status == "Approved" ? "#3F6449" : record.status == "Declined" ? "#7A263A" : "#D6A21E";
    ui->lblStatus->setStyleSheet(QString("font-family:Inter; font-size:11px; color:%1; background:%2; border:none; border-radius:14px;").arg(color,fill));
    ui->lblStudentID->setText(record.studentId);
    ui->lblStudentName->setText(record.studentName);
    ui->lblMedicalID->setText(record.medicalId);
    ui->lblReason->setText(record.reason);
    ui->lblReason->setToolTip(record.reason);
    ui->lblMedicalPeriod->setText(record.fromDate.toString("dd/MM/yyyy") + QString::fromUtf8(" — ") + record.toDate.toString("dd/MM/yyyy"));
    for (const auto &course : record.courses) {
        const int row = ui->tblCourses->rowCount();
        ui->tblCourses->insertRow(row);
        ui->tblCourses->setItem(row,0,new QTableWidgetItem(course.name));
        ui->tblCourses->setItem(row,1,new QTableWidgetItem(course.code));
        ui->tblCourses->setSpan(row,0,1,2);
        auto *line = new QLabel(ui->tblCourses);
        line->setObjectName(QString("courseLine%1").arg(row));
        line->setTextFormat(Qt::PlainText);
        line->setText(course.name + QString::fromUtf8(" — ") + course.code);
        line->setStyleSheet("font-family:Inter; font-size:13px; color:#303333; background:#FFFDF8; border:none;");
        ui->tblCourses->setCellWidget(row,0,line);
    }
    ui->btnApprove->setEnabled(record.status == "Pending");
    return true;
}
void ApplicationDetails::refreshData() { if (AppSession::isAdmin()) loadData(true); }
void ApplicationDetails::onBackClicked() { if (AppSession::isAdmin()) close(); }
void ApplicationDetails::onApproveClicked()
{
    if (!AppSession::isAdmin() || currentNumericId <= 0) return;
    const int id = Database::applicationId(currentApplicationID);
    if (id <= 0) return;
    if (QMessageBox::question(this,"Approve Application",QString("Approve %1?").arg(currentApplicationID),
                             QMessageBox::Yes | QMessageBox::No,QMessageBox::No) != QMessageBox::Yes) return;
    if (!AppSession::isAdmin()) return;
    const bool saved = Database::reviewApplication(id,"Approved");
    const QString error = saved ? QString() : Database::error();
    loadData(false);
    if (saved) emit statusChanged(Database::applicationCode(id),"Approved");
    else QMessageBox::critical(this,"Approval failed",error);
}
