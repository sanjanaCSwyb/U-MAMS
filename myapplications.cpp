#include "myapplications.h"
#include "app_session.h"
#include "database.h"
#include <QLabel>
#include <QSignalBlocker>
#include "figma_ui.h"
#include "ui_myapplications.h"
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QDate>


#include <QMessageBox>
#include <QColor>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
void showStudentDetails(QWidget *parent, const QString &applicationID,
                        const QString &studentID, const QString &studentName,
                        const QString &medicalID, const QString &fromDate,
                        const QString &toDate, const QString &status,
                        const QString &reason, const QString &courses)
{
    QDialog details(parent);
    if (!AppSession::protect(&details, AppSession::Role::Student)) return;
    details.setObjectName("StudentApplicationDetails");
    details.setWindowTitle("U-MAMS - Application Details");
    FigmaUi::configureWindowControls(&details);
    details.setMinimumSize(480, 500);
    details.resize(720, 720);
    details.setStyleSheet(R"(
        QDialog#StudentApplicationDetails { background:#F7F1E3; color:#263238; }
        QLabel {
            font-family:Inter; font-size:14px; color:#263238;
            background:transparent; border:none; padding:0px;
        }
        QFrame#detailsHeader { background:#7A263A; border:none; }
        QLabel#detailsTitle { color:#FFFDF8; font-size:24px; font-weight:600; }
        QLabel#applicationIdValue { color:#FFFDF8; font-size:16px; font-weight:500; }
        QLabel#detailsSubtitle { color:#FFFDF8; font-size:12px; }
        QWidget#detailsBody, QScrollArea#detailsScroll { background:#F7F1E3; border:none; }
        QScrollBar:vertical { background:#EEE6D6; width:10px; margin:4px 1px; }
        QScrollBar::handle:vertical { background:#BCA98F; min-height:32px; border-radius:4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:transparent; }
        QFrame#detailsInfoCard, QFrame#detailsReasonCard, QFrame#detailsCoursesCard {
            background:#FFFDF8; border:1px solid #DBD1BA; border-radius:10px;
        }
        QLabel[detailsCaption="true"] {
            color:#52616B; font-size:12px; font-weight:500;
        }
        QPushButton#btnCloseDetails {
            font-family:Inter; font-size:14px; color:#FFFDF8; background:#7A263A;
            border:2px solid #7A263A; border-radius:8px; padding:10px 24px;
        }
        QPushButton#btnCloseDetails:hover, QPushButton#btnCloseDetails:focus {
            border-color:#D6A21E;
        }
    )");

    auto *layout = new QVBoxLayout(&details);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *header = new QFrame(&details);
    header->setObjectName("detailsHeader");
    auto *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(24, 20, 24, 20);
    headerLayout->setSpacing(10);
    auto *title = new QLabel("Application Details", header);
    title->setObjectName("detailsTitle");
    headerLayout->addWidget(title);
    auto *summary = new QHBoxLayout;
    auto *application = new QLabel(applicationID, header);
    application->setObjectName("applicationIdValue");
    application->setTextFormat(Qt::PlainText);
    application->setTextInteractionFlags(Qt::TextSelectableByMouse);
    summary->addWidget(application);
    summary->addStretch();
    auto *badge = new QLabel(status, header);
    badge->setObjectName("applicationStatusValue");
    badge->setAlignment(Qt::AlignCenter);
    badge->setMinimumSize(110, 30);
    const QString foreground = status == "Approved" ? "#215239" : status == "Declined" ? "#7A263A" : "#715200";
    const QString background = status == "Approved" ? "#E4F2E7" : status == "Declined" ? "#F8E5E9" : "#FFF2C5";
    badge->setStyleSheet(QString("color:%1;background:%2;border:none;border-radius:8px;padding:4px 12px;font-size:13px;font-weight:600;").arg(foreground, background));
    summary->addWidget(badge);
    headerLayout->addLayout(summary);
    auto *subtitle = new QLabel("Your submitted medical application", header);
    subtitle->setObjectName("detailsSubtitle");
    headerLayout->addWidget(subtitle);
    layout->addWidget(header);

    auto *scroll = new QScrollArea(&details);
    scroll->setObjectName("detailsScroll");
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto *body = new QWidget;
    body->setObjectName("detailsBody");
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(24, 20, 24, 20);
    bodyLayout->setSpacing(16);
    auto caption = [](const QString &text, QWidget *owner) {
        auto *label = new QLabel(text, owner);
        label->setProperty("detailsCaption", true);
        return label;
    };
    auto value = [](const QString &name, const QString &text, QWidget *owner) {
        auto *label = new QLabel(text, owner);
        label->setObjectName(name);
        label->setTextFormat(Qt::PlainText);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        label->setWordWrap(true);
        label->setMinimumHeight(22);
        return label;
    };
    auto *info = new QFrame(body);
    info->setObjectName("detailsInfoCard");
    auto *grid = new QGridLayout(info);
    grid->setContentsMargins(20, 16, 20, 16);
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(8);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    grid->addWidget(caption("Student ID", info), 0, 0);
    grid->addWidget(caption("Student name", info), 0, 1);
    grid->addWidget(value("studentIdValue", studentID, info), 1, 0);
    grid->addWidget(value("studentNameValue", studentName, info), 1, 1);
    grid->addWidget(caption("Medical ID", info), 2, 0, 1, 2);
    grid->addWidget(value("medicalIdValue", medicalID, info), 3, 0, 1, 2);
    grid->addWidget(caption("Medical period - From", info), 4, 0);
    grid->addWidget(caption("Medical period - To", info), 4, 1);
    grid->addWidget(value("periodFromValue", fromDate, info), 5, 0);
    grid->addWidget(value("periodToValue", toDate, info), 5, 1);
    bodyLayout->addWidget(info);
    for (const auto &section : {qMakePair(QString("Reason for absence"), reason), qMakePair(QString("Missed courses"), courses)}) {
        const bool isReason = section.first == "Reason for absence";
        auto *card = new QFrame(body);
        card->setObjectName(isReason ? "detailsReasonCard" : "detailsCoursesCard");
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 16, 20, 16);
        cardLayout->setSpacing(10);
        cardLayout->addWidget(caption(section.first, card));
        cardLayout->addWidget(value(isReason ? "reasonValue" : "coursesValue", section.second, card));
        bodyLayout->addWidget(card);
    }
    bodyLayout->addStretch();
    scroll->setWidget(body);
    layout->addWidget(scroll, 1);
    auto *footer = new QHBoxLayout;
    footer->setContentsMargins(24, 16, 24, 20);
    footer->addStretch();
    auto *close = new QPushButton("Close", &details);
    close->setObjectName("btnCloseDetails");
    close->setMinimumWidth(130);
    close->setDefault(true);
    QObject::connect(close, &QPushButton::clicked, &details, &QDialog::accept);
    footer->addWidget(close);
    layout->addLayout(footer);
    details.exec();
}

class SubmittedTableDelegate final : public QStyledItemDelegate {
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
        if ((index.column()==2 || index.column()==3)) {
            const QDate date=QDate::fromString(index.data().toString(), "dd/MM/yyyy");
            if (date.isValid()) option->text=date.toString("dd/MM/yy");
        }

    }
};
}

MyApplications::MyApplications(QWidget *parent)
    : QDialog(parent), ui(new Ui::MyApplications)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Student)) return;
    setWindowTitle("U-MAMS - My Medical Applications");
    ui->tblApplications->setRowCount(0);
    ui->tblApplications->setColumnCount(5);
    ui->tblApplications->setHorizontalHeaderLabels({"Application ID","Medical ID","From","To","Status"});
    ui->tblApplications->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblApplications->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblApplications->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblApplications->horizontalHeader()->setStretchLastSection(true);
    ui->tblApplications->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblApplications->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const int widths[]={180,240,120,140,188};
    for (int column=0; column<5; ++column) ui->tblApplications->setColumnWidth(column,widths[column]);
    ui->tblApplications->verticalHeader()->setDefaultSectionSize(80);
    ui->tblApplications->setItemDelegate(new SubmittedTableDelegate(ui->tblApplications));
    auto *emptyLabel=new QLabel(ui->tblApplications->viewport());
    emptyLabel->setObjectName("applicationsEmptyState");
    emptyLabel->setTextFormat(Qt::PlainText);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setWordWrap(true);
    emptyLabel->setStyleSheet("font-family:Inter;font-size:14px;color:#52616B;background:transparent;border:none;padding:24px;");
    auto *emptyLayout=new QVBoxLayout(ui->tblApplications->viewport());
    emptyLayout->setContentsMargins(0,0,0,0);
    emptyLayout->addWidget(emptyLabel);
    connect(ui->btnViewDetails,&QPushButton::clicked,this,&MyApplications::onViewDetailsClicked);
    connect(ui->btnBack,&QPushButton::clicked,this,&MyApplications::onBackClicked);
    FigmaUi::install(this);
    refreshData();
}

MyApplications::~MyApplications() { delete ui; }

void MyApplications::refreshData()
{
    if (!AppSession::isStudent()) return;
    const QSignalBlocker block(ui->tblApplications);
    ui->tblApplications->clearContents();
    ui->tblApplications->setRowCount(0);
    ui->btnViewDetails->setEnabled(false);
    auto *emptyLabel=ui->tblApplications->viewport()->findChild<QLabel *>("applicationsEmptyState");
    QList<ApplicationRecord> applications;
    if (!Database::applications(applications,AppSession::principal())) {
        if (emptyLabel) {
            emptyLabel->setText("Your applications could not be loaded. Please reopen this page to try again.");
            emptyLabel->setToolTip(Database::error());
            emptyLabel->show();
        }
        QMessageBox::warning(this,"Unable to Load Applications",Database::error());
        return;
    }
    for (const auto &application:applications) {
        const int row=ui->tblApplications->rowCount();
        ui->tblApplications->insertRow(row);
        auto *id=new QTableWidgetItem(Database::applicationCode(application.id));
        id->setData(Qt::UserRole,application.id);
        ui->tblApplications->setItem(row,0,id);
        ui->tblApplications->setItem(row,1,new QTableWidgetItem(application.medicalId));
        ui->tblApplications->setItem(row,2,new QTableWidgetItem(application.fromDate.toString("dd/MM/yyyy")));
        ui->tblApplications->setItem(row,3,new QTableWidgetItem(application.toDate.toString("dd/MM/yyyy")));
        auto *status=new QTableWidgetItem(application.status);
        status->setForeground(QColor(application.status=="Approved" ? "#3F6449" : application.status=="Declined" ? "#7A263A" : "#D6A21E"));
        ui->tblApplications->setItem(row,4,status);
    }
    if (emptyLabel) {
        emptyLabel->setText("You have not submitted any medical applications yet.");
        emptyLabel->setToolTip(QString());
        emptyLabel->setVisible(applications.isEmpty());
    }
    ui->btnViewDetails->setEnabled(!applications.isEmpty());
}

void MyApplications::onViewDetailsClicked()
{
    if (!AppSession::isStudent()) return;
    const int row=ui->tblApplications->currentRow();
    auto *id=row>=0 ? ui->tblApplications->item(row,0) : nullptr;
    if (!id) {
        QMessageBox::warning(this,"No Application Selected","Please select an application first.");
        return;
    }
    ApplicationRecord application;
    if (!Database::application(id->data(Qt::UserRole).toInt(),application)) {
        QMessageBox::warning(this,"Unable to Load Application",Database::error());
        return;
    }
    if (application.studentId!=AppSession::principal()) return;
    QStringList courses;
    for (const auto &course:application.courses)
        courses << QStringLiteral("• %1 (%2)").arg(course.name,course.code);
    showStudentDetails(this,Database::applicationCode(application.id),application.studentId,
                       application.studentName,application.medicalId,
                       application.fromDate.toString("dd/MM/yyyy"),application.toDate.toString("dd/MM/yyyy"),
                       application.status,application.reason,courses.join('\n'));
    refreshData();
}

void MyApplications::onBackClicked() { close(); }
