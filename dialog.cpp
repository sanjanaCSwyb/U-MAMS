#include "app_session.h"
#include "dialog.h"
#include "database.h"
#include "figma_ui.h"
#include "ui_dialog.h"
#include "applicationdetails.h"
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QStyle>
#include <QDate>


#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QLabel>
#include <QPainter>
#include <QStyle>


namespace {
class RoundedStatusBadge final : public QLabel {
public:
    explicit RoundedStatusBadge(QWidget *parent=nullptr) : QLabel(parent) {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_TranslucentBackground);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QString status=property("status").toString();
        const QColor fill=status == "Approved" ? QColor("#3F6449") :
                          status == "Declined" ? QColor("#7A263A") : QColor("#D6A21E");
        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(QRectF(0,0,width(),height()),height()/2.0,height()/2.0);
        painter.setFont(font());
        painter.setPen(Qt::white);
        painter.drawText(rect(),Qt::AlignCenter,text());
    }
};

class ApplicationTableDelegate final : public QStyledItemDelegate {
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
        if ((index.column()==3 || index.column()==4)) {
            const QDate date=QDate::fromString(index.data().toString(), "dd/MM/yyyy");
            if (date.isValid()) option->text=date.toString("dd/MM/yy");
        }
        if (index.column()==5) option->text.clear();
    }
};
}

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Admin)) return;
    setWindowTitle("U-MAMS - View Applications");
    ui->cmbStatus->clear();
    ui->cmbStatus->addItems({"All statuses", "Pending", "Approved", "Declined"});
    ui->tblApplications->setColumnCount(6);
    ui->tblApplications->setHorizontalHeaderLabels({"Application ID", "Student ID", "Student Name", "From", "To", "Status"});
    ui->tblApplications->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tblApplications->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblApplications->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tblApplications->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tblApplications->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    const int widths[] = {150,120,170,110,110,208};
    for (int column=0; column<6; ++column) ui->tblApplications->setColumnWidth(column, widths[column]);
    ui->tblApplications->verticalHeader()->setDefaultSectionSize(78);
    ui->tblApplications->setItemDelegate(new ApplicationTableDelegate(ui->tblApplications));
    connect(ui->btnSearch, &QPushButton::clicked, this, &Dialog::onSearchClicked);
    connect(ui->cmbStatus, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Dialog::onStatusChanged);
    connect(ui->btnApprove, &QPushButton::clicked, this, &Dialog::onApproveClicked);
    connect(ui->btnDecline, &QPushButton::clicked, this, &Dialog::onDeclineClicked);
    connect(ui->btnViewDetails, &QPushButton::clicked, this, &Dialog::onViewDetailsClicked);
    connect(ui->btnBack, &QPushButton::clicked, this, &Dialog::onBackClicked);
    connect(ui->tblApplications, &QTableWidget::itemSelectionChanged, this, [this] {
        const int row = ui->tblApplications->currentRow();
        const auto *item = row >= 0 ? ui->tblApplications->item(row,5) : nullptr;
        const bool selected = AppSession::isAdmin() && item;
        const bool pending = selected && item->text() == "Pending";
        ui->btnViewDetails->setEnabled(selected);
        ui->btnApprove->setEnabled(pending);
        ui->btnDecline->setEnabled(pending);
    });
    refreshData();
    FigmaUi::install(this);
}

Dialog::~Dialog() { delete ui; }

int Dialog::selectedApplicationId() const
{
    const int row = ui->tblApplications->currentRow();
    const auto *item = row >= 0 ? ui->tblApplications->item(row,0) : nullptr;
    return item ? item->data(Qt::UserRole).toInt() : 0;
}

bool Dialog::loadData(bool reportError)
{
    if (!AppSession::isAdmin()) return false;
    const int previous = selectedApplicationId();
    ui->tblApplications->setRowCount(0);
    ui->btnViewDetails->setEnabled(false);
    ui->btnApprove->setEnabled(false);
    ui->btnDecline->setEnabled(false);
    QList<ApplicationRecord> records;
    const QString status = ui->cmbStatus->currentIndex() > 0 ? ui->cmbStatus->currentText() : QString();
    if (!Database::applications(records, ui->txtSearchStudentID->text().trimmed(), status)) {
        ui->lblTitle->setText("Medical Applications - unable to load");
        if (reportError) QMessageBox::critical(this, "Applications", Database::error());
        return false;
    }
    ui->lblTitle->setText(records.isEmpty() ? "Medical Applications - no matching applications" : "Medical Applications");
    for (const auto &record : records) {
        const int row = ui->tblApplications->rowCount();
        ui->tblApplications->insertRow(row);
        const QStringList values = {Database::applicationCode(record.id), record.studentId, record.studentName,
            record.fromDate.toString("dd/MM/yyyy"), record.toDate.toString("dd/MM/yyyy"), record.status};
        for (int column=0; column<values.size(); ++column) {
            auto *item = new QTableWidgetItem(values[column]);
            if (column == 0) item->setData(Qt::UserRole, record.id);
            ui->tblApplications->setItem(row,column,item);
        }
        auto *cell = new QWidget(ui->tblApplications);
        cell->setObjectName(QString("statusCell%1").arg(record.id));
        cell->setStyleSheet("background: transparent; border: none;");
        auto *badge = new RoundedStatusBadge(cell);
        badge->setObjectName("statusBadge");
        badge->setGeometry(0,25,100,28);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet("font-family:Inter; font-size:12px; color:white; background:transparent; border:none;");
        badge->setText(record.status);
        badge->setProperty("status", record.status);
        ui->tblApplications->setCellWidget(row,5,cell);
        if (record.id == previous) ui->tblApplications->selectRow(row);
    }
    return true;
}

void Dialog::refreshData() { if (AppSession::isAdmin()) loadData(true); }
void Dialog::onSearchClicked() { if (AppSession::isAdmin()) refreshData(); }
void Dialog::onStatusChanged(int index) { Q_UNUSED(index); if (AppSession::isAdmin()) refreshData(); }
void Dialog::onApproveClicked() { if (AppSession::isAdmin()) reviewSelected("Approved"); }
void Dialog::onDeclineClicked() { if (AppSession::isAdmin()) reviewSelected("Declined"); }

void Dialog::reviewSelected(const QString &status)
{
    if (!AppSession::isAdmin()) return;
    const int id = selectedApplicationId();
    if (id <= 0) { QMessageBox::warning(this, "Application", "Select an application first."); return; }
    ApplicationRecord record;
    if (!Database::application(id, record)) { const QString error=Database::error(); loadData(false); QMessageBox::critical(this,"Application",error); return; }
    if (record.status != "Pending") { loadData(false); QMessageBox::information(this,"Application","Only pending applications can be reviewed."); return; }
    if (QMessageBox::question(this, "Review Application", QString("Mark %1 as %2?").arg(Database::applicationCode(id),status),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
    if (!AppSession::isAdmin()) return;
    const bool saved = Database::reviewApplication(id,status);
    const QString error = saved ? QString() : Database::error();
    loadData(false);
    if (!saved) QMessageBox::critical(this,"Review failed",error);
}

void Dialog::onViewDetailsClicked()
{
    if (!AppSession::isAdmin()) return;
    const int id = selectedApplicationId();
    if (id <= 0) { QMessageBox::warning(this,"Application","Select an application first."); return; }
    ApplicationDetails details(this);
    connect(&details, &ApplicationDetails::statusChanged, this, [this](const QString &, const QString &) { refreshData(); });
    if (details.setApplicationId(id)) details.exec();
    refreshData();
}

void Dialog::onBackClicked() { if (AppSession::isAdmin()) close(); }
