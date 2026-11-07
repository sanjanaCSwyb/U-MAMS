#include "monthlyapprovedmedicals.h"
#include "ui_monthlyapprovedmedicals.h"
#include "app_session.h"
#include "database.h"
#include "figma_ui.h"

#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMap>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QScrollArea>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTimeZone>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {
QString displayDate(const QDate &date)
{
    return date.isValid() ? QLocale(QLocale::English).toString(date, "dd MMM yyyy")
                          : QStringLiteral("Not available");
}

QString displayPeriod(const ApplicationRecord &record)
{
    return displayDate(record.fromDate) + QStringLiteral(" — ") + displayDate(record.toDate);
}

QString displayTimestamp(const QDateTime &timestamp, const QString &empty)
{
    if (!timestamp.isValid()) return empty;
    return QLocale(QLocale::English).toString(timestamp.toTimeZone(QTimeZone("Asia/Colombo")),
                                            "dd MMM yyyy, HH:mm:ss") + QStringLiteral(" (Asia/Colombo)");
}

QLabel *detailValue(const QString &name, const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(name);
    label->setTextFormat(Qt::PlainText);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    label->setMinimumHeight(22);
    return label;
}

void showReadOnlyDetails(QWidget *parent, const ApplicationRecord &record)
{
    if (!AppSession::isAdmin()) return;
    QDialog details(parent);
    if (!AppSession::protect(&details, AppSession::Role::Admin)) return;
    details.setObjectName("MonthlyApprovedMedicalDetails");
    details.setWindowTitle("U-MAMS - Medical Application Details");
    details.setMinimumSize(600, 500);
    details.resize(820, 760);
    FigmaUi::configureWindowControls(&details);
    details.setStyleSheet(R"(
        QDialog#MonthlyApprovedMedicalDetails { background:#F7F1E3; color:#263238; }
        QLabel { font-family:Inter; font-size:14px; color:#263238; background:transparent; border:none; }
        QFrame#detailsHeader { background:#7A263A; border:none; }
        QLabel#detailsTitle { font-size:24px; font-weight:600; color:#FFFDF8; }
        QLabel#detailsSubtitle { font-size:12px; color:#FFFDF8; }
        QWidget#detailsBody, QScrollArea#detailsScroll { background:#F7F1E3; border:none; }
        QFrame#detailsInfoCard, QFrame#detailsReasonCard, QFrame#detailsCoursesCard {
            background:#FFFDF8; border:1px solid #DBD1BA; border-radius:10px;
        }
        QLabel[detailsCaption="true"] { font-size:12px; font-weight:500; color:#52616B; }
        QPlainTextEdit { font-family:Inter; font-size:14px; color:#263238; background:#FFFDF8;
            border:1px solid #DBD1BA; border-radius:6px; padding:10px;
            selection-background-color:#7A263A; selection-color:#FFFDF8; }
        QScrollBar:vertical { background:#EEE6D6; width:10px; margin:2px; }
        QScrollBar::handle:vertical { background:#BCA98F; min-height:32px; border-radius:4px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0px; }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background:transparent; }
        QPushButton#btnCloseDetails { font-family:Inter; font-size:14px; font-weight:500; color:#FFFDF8;
            background:#7A263A; border:2px solid #7A263A; border-radius:8px; padding:10px 24px; }
        QPushButton#btnCloseDetails:hover, QPushButton#btnCloseDetails:focus { border-color:#D6A21E; }
    )");

    auto *layout = new QVBoxLayout(&details);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto *header = new QFrame(&details);
    header->setObjectName("detailsHeader");
    auto *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(24, 20, 24, 20);
    headerLayout->setSpacing(6);
    auto *title = new QLabel("Medical Application Details", header);
    title->setObjectName("detailsTitle");
    headerLayout->addWidget(title);
    auto *subtitle = new QLabel("Read-only application record", header);
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
        label->setTextFormat(Qt::PlainText);
        label->setProperty("detailsCaption", true);
        return label;
    };
    auto *info = new QFrame(body);
    info->setObjectName("detailsInfoCard");
    auto *grid = new QGridLayout(info);
    grid->setContentsMargins(20, 16, 20, 16);
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(9);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    auto addPair = [&](int row, int column, const QString &titleText,
                       const QString &name, const QString &text) {
        grid->addWidget(caption(titleText, info), row, column);
        grid->addWidget(detailValue(name, text, info), row + 1, column);
    };
    addPair(0, 0, "Application ID", "applicationIdValue", Database::applicationCode(record.id));
    addPair(0, 1, "Status", "statusValue", record.status);
    addPair(2, 0, "Student name", "studentNameValue", record.studentName);
    addPair(2, 1, "Registration number", "registrationNumberValue", record.studentId);
    addPair(4, 0, "Medical ID", "medicalIdValue", record.medicalId);
    grid->addWidget(caption("Full medical period", info), 6, 0, 1, 2);
    grid->addWidget(detailValue("periodValue", displayPeriod(record), info), 7, 0, 1, 2);
    grid->addWidget(caption("Submitted", info), 8, 0, 1, 2);
    grid->addWidget(detailValue("submittedValue", displayTimestamp(record.submittedAt, "Not available"), info), 9, 0, 1, 2);
    grid->addWidget(caption("Reviewed", info), 10, 0, 1, 2);
    grid->addWidget(detailValue("reviewedValue", displayTimestamp(record.reviewedAt, "Not reviewed"), info), 11, 0, 1, 2);
    bodyLayout->addWidget(info);

    QStringList courses;
    for (const auto &course : record.courses)
        courses.append(QStringLiteral("%1 (%2)").arg(course.name, course.code));
    auto addTextSection = [&](const QString &titleText, const QString &cardName,
                              const QString &fieldName, const QString &text) {
        auto *card = new QFrame(body);
        card->setObjectName(cardName);
        auto *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(20, 16, 20, 16);
        cardLayout->setSpacing(10);
        cardLayout->addWidget(caption(titleText, card));
        auto *field = new QPlainTextEdit(card);
        field->setObjectName(fieldName);
        field->setReadOnly(true);
        field->setPlainText(text);
        field->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        field->setMinimumHeight(150);
        field->setMaximumHeight(190);
        field->setAccessibleName(titleText);
        cardLayout->addWidget(field);
        bodyLayout->addWidget(card);
    };
    addTextSection("Reason for absence", "detailsReasonCard", "reasonValue", record.reason);
    addTextSection("Missed courses", "detailsCoursesCard", "coursesValue",
                   courses.isEmpty() ? "No courses recorded." : courses.join('\n'));
    bodyLayout->addStretch();
    scroll->setWidget(body);
    layout->addWidget(scroll, 1);

    auto *footer = new QHBoxLayout;
    footer->setContentsMargins(24, 16, 24, 20);
    footer->addStretch();
    auto *close = new QPushButton("CLOSE", &details);
    close->setObjectName("btnCloseDetails");
    close->setMinimumSize(140, 46);
    close->setDefault(true);
    QObject::connect(close, &QPushButton::clicked, &details, &QDialog::accept);
    footer->addWidget(close);
    layout->addLayout(footer);
    details.exec();
}
}

MonthlyApprovedMedicals::MonthlyApprovedMedicals(QWidget *parent)
    : QDialog(parent), ui(new Ui::MonthlyApprovedMedicals)
{
    ui->setupUi(this);
    if (!AppSession::protect(this, AppSession::Role::Admin)) return;
    FigmaUi::loadFont();
    FigmaUi::configureWindowControls(this);
    // Native filter arrows use ButtonText, which can remain white when the
    // system uses a dark palette even though the stylesheet sets dark text.
    QPalette filterPalette = ui->spnYear->palette();
    for (const auto role : {QPalette::ButtonText, QPalette::WindowText, QPalette::Text})
        filterPalette.setColor(QPalette::All, role, QColor("#263238"));
    for (const auto role : {QPalette::Button, QPalette::Base, QPalette::Window})
        filterPalette.setColor(QPalette::All, role, QColor("#FFFDF8"));
    filterPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#9B8D74"));
    ui->spnYear->setPalette(filterPalette);
    ui->cmbMonth->setPalette(filterPalette);
    const QPixmap crest(":/assets/wayamba_logo.png");
    ui->headerLogo->setPixmap(crest.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    const QLocale english(QLocale::English);
    for (int month = 1; month <= 12; ++month)
        ui->cmbMonth->addItem(english.monthName(month, QLocale::LongFormat), month);
    const QDate today = QDateTime::currentDateTimeUtc().toTimeZone(QTimeZone("Asia/Colombo")).date();
    ui->cmbMonth->setCurrentIndex(today.month() - 1);
    ui->spnYear->setValue(today.year());
    ui->treeApplications->header()->setStretchLastSection(false);
    ui->treeApplications->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeApplications->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->treeApplications->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeApplications->setAllColumnsShowFocus(true);
    ui->treeApplications->setIndentation(22);

    const auto updateYearButtons = [this](int year) {
        ui->btnYearPrevious->setEnabled(year > ui->spnYear->minimum());
        ui->btnYearNext->setEnabled(year < ui->spnYear->maximum());
    };
    connect(ui->btnYearPrevious, &QToolButton::clicked, ui->spnYear, &QSpinBox::stepDown);
    connect(ui->btnYearNext, &QToolButton::clicked, ui->spnYear, &QSpinBox::stepUp);
    connect(ui->spnYear, &QSpinBox::valueChanged, this, updateYearButtons);
    updateYearButtons(ui->spnYear->value());
    connect(ui->cmbMonth, &QComboBox::currentIndexChanged, this, [this](int) { refreshData(); });
    connect(ui->spnYear, &QSpinBox::valueChanged, this, [this](int) { refreshData(); });
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MonthlyApprovedMedicals::refreshData);
    connect(ui->btnBack, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnViewDetails, &QPushButton::clicked, this, &MonthlyApprovedMedicals::queueSelectedDetails);
    connect(ui->treeApplications, &QTreeWidget::currentItemChanged, this,
            [this](QTreeWidgetItem *, QTreeWidgetItem *) { updateSelection(); });
    connect(ui->treeApplications, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        if (item && item->data(0, Qt::UserRole).toInt() > 0) queueSelectedDetails();
    });
    // Mouse click already opens a record. Separate keyboard shortcuts avoid
    // connecting the double-click activation signal to a second popup.
    for (const int key : {Qt::Key_Return, Qt::Key_Enter}) {
        auto *shortcut = new QShortcut(QKeySequence(key), ui->treeApplications);
        shortcut->setContext(Qt::WidgetShortcut);
        connect(shortcut, &QShortcut::activated, this, &MonthlyApprovedMedicals::queueSelectedDetails);
    }
    refreshData();
}

MonthlyApprovedMedicals::~MonthlyApprovedMedicals()
{
    delete ui;
}

void MonthlyApprovedMedicals::refreshData()
{
    if (loading) return;
    QScopedValueRollback<bool> refreshing(loading, true);
    const QSignalBlocker treeSignals(ui->treeApplications);
    ui->treeApplications->clear();
    ui->btnViewDetails->setEnabled(false);
    ui->resultsStack->setCurrentWidget(ui->pageState);
    ui->lblSummary->setText("Approved medicals");
    if (!AppSession::isAdmin()) {
        ui->lblState->setText("This page is available to administrators.");
        AppSession::deny(this);
        return;
    }
    ui->btnRefresh->setEnabled(false);
    const QDate month(ui->spnYear->value(), ui->cmbMonth->currentData().toInt(), 1);
    const QString monthTitle = QLocale(QLocale::English).toString(month, "MMMM yyyy");
    QList<ApplicationRecord> records;
    const bool loaded = Database::monthlyApprovedApplications(month, records);
    ui->btnRefresh->setEnabled(true);
    if (!loaded) {
        const QString error = Database::error();
        ui->lblState->setText(error.isEmpty()
            ? tr("Approved medicals could not be loaded. Please refresh to try again.")
            : tr("Approved medicals could not be loaded.\n%1").arg(error));
        return;
    }
    const QString countText = records.size() == 1 ? tr("1 approved medical")
                                                 : tr("%1 approved medicals").arg(records.size());
    ui->lblSummary->setText(countText + QStringLiteral(" · ") + monthTitle);
    if (records.isEmpty()) {
        ui->lblState->setText(tr("No approved medical periods overlap %1.").arg(monthTitle));
        return;
    }

    QMap<QString, QList<ApplicationRecord>> categories;
    for (const auto &record : records)
        categories[Database::registrationCategory(record.studentId)].append(record);
    for (auto category = categories.cbegin(); category != categories.cend(); ++category) {
        auto *group = new QTreeWidgetItem(ui->treeApplications);
        group->setText(0, category.key());
        group->setText(1, category.value().size() == 1 ? tr("1 approved medical")
                                                     : tr("%1 approved medicals").arg(category.value().size()));
        group->setData(0, Qt::UserRole, 0);
        QFont font = group->font(0);
        font.setBold(true);
        for (int column = 0; column < 3; ++column) {
            group->setFont(column, font);
            group->setForeground(column, QColor("#7A263A"));
            group->setBackground(column, QColor("#F7F1E3"));
        }
        for (const auto &record : category.value()) {
            auto *child = new QTreeWidgetItem(group);
            child->setText(0, record.studentId);
            child->setText(1, displayPeriod(record));
            child->setToolTip(1, displayPeriod(record));
            child->setText(2, Database::applicationCode(record.id));
            child->setData(0, Qt::UserRole, record.id);
            child->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        }
        group->setExpanded(true);
    }
    ui->resultsStack->setCurrentWidget(ui->pageApplications);
}

void MonthlyApprovedMedicals::updateSelection()
{
    const auto *item = ui->treeApplications->currentItem();
    ui->btnViewDetails->setEnabled(AppSession::isAdmin() && item && item->data(0, Qt::UserRole).toInt() > 0);
}

void MonthlyApprovedMedicals::queueSelectedDetails()
{
    if (!AppSession::isAdmin() || detailsOpen || detailsQueued) return;
    const auto *item = ui->treeApplications->currentItem();
    if (!item || item->data(0, Qt::UserRole).toInt() <= 0) return;
    detailsQueued = true;
    QTimer::singleShot(0, this, [this] {
        detailsQueued = false;
        openSelectedDetails();
    });
}

void MonthlyApprovedMedicals::openSelectedDetails()
{
    if (!AppSession::isAdmin() || detailsOpen) return;
    const auto *item = ui->treeApplications->currentItem();
    const int id = item ? item->data(0, Qt::UserRole).toInt() : 0;
    if (id <= 0) return;
    QScopedValueRollback<bool> opening(detailsOpen, true);
    ApplicationRecord record;
    if (!Database::application(id, record)) {
        ui->treeApplications->clear();
        ui->btnViewDetails->setEnabled(false);
        ui->resultsStack->setCurrentWidget(ui->pageState);
        ui->lblSummary->setText("Approved medicals");
        ui->lblState->setText("The selected application could not be loaded. Please refresh to try again.");
        QMessageBox::warning(this, "Unable to Load Application", Database::error());
        return;
    }
    if (!AppSession::isAdmin()) return;
    showReadOnlyDetails(this, record);
    refreshData();
}
