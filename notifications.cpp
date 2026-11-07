#include "notifications.h"
#include "app_session.h"
#include "database.h"
#include "ui_notifications.h"
#include "figma_ui.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace {
QLabel *notificationText(const QString &text, QWidget *parent, const QString &style)
{
    auto *label = new QLabel(parent);
    label->setTextFormat(Qt::PlainText);
    label->setText(text);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setStyleSheet(style);
    return label;
}

const QString bodyStyle = QStringLiteral(
    "font-family:Inter;font-size:13px;color:#263238;background:transparent;border:none;");
const QString mutedStyle = QStringLiteral(
    "font-family:Inter;font-size:11px;color:#6F726E;background:transparent;border:none;");
const QString titleStyle = QStringLiteral(
    "font-family:Inter;font-size:16px;font-weight:500;color:#263238;"
    "background:transparent;border:none;");
}

Notifications::Notifications(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Notifications)
{
    ui->setupUi(this);
    if (!AppSession::protectAuthenticated(this)) return;
    setWindowTitle(tr("U-MAMS - Notifications"));

    // Hide and clear the Designer placeholders before loading account-owned rows.
    for (auto *label : {
             ui->lblNotificationTitle, ui->lblNotificationMessage, ui->lblNotificationTime,
             ui->lblNotificationTitle2, ui->lblNotificationMessage2, ui->lblNotificationTime2,
             ui->lblNotificationTitle3, ui->lblNotificationMessage3, ui->lblNotificationTime3}) {
        label->clear();
    }
    ui->notificationCard1->hide();
    ui->notificationCard2->hide();
    ui->notificationCard3->hide();

    notificationList = new QScrollArea(ui->figmaCanvas);
    notificationList->setObjectName("databaseNotifications");
    notificationList->setGeometry(260, 185, 870, 500);
    notificationList->setFrameShape(QFrame::NoFrame);
    notificationList->setWidgetResizable(true);
    notificationList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    notificationList->setStyleSheet(
        "QScrollArea#databaseNotifications{background:transparent;border:none;}"
        "QScrollArea#databaseNotifications>QWidget>QWidget{background:transparent;}"
        "QScrollBar:vertical{background:#EEE8DA;width:9px;border:none;}"
        "QScrollBar::handle:vertical{background:#B8AD98;border-radius:4px;min-height:30px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0px;}");

    auto *refreshButton = new QPushButton(tr("Refresh"), ui->figmaCanvas);
    refreshButton->setObjectName("refreshNotifications");
    refreshButton->setGeometry(970, 115, 160, 40);
    refreshButton->setAutoDefault(false);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(
        "QPushButton#refreshNotifications{font-family:Inter;font-size:12px;color:#30271B;"
        "background:#D6A21E;border:none;border-radius:7px;padding:0px;}"
        "QPushButton#refreshNotifications:hover{background:#E1B43F;}");
    connect(refreshButton, &QPushButton::clicked, this, &Notifications::refreshData);
    connect(ui->btnBack, &QPushButton::clicked, this, &Notifications::onBackClicked);

    refreshData();
    FigmaUi::install(this);
}

Notifications::~Notifications()
{
    delete ui;
}

void Notifications::refreshData()
{
    if (!AppSession::protectAuthenticated(this) || !notificationList) return;

    auto *content = new QWidget;
    content->setObjectName("notificationRecords");
    content->setStyleSheet("QWidget#notificationRecords{background:transparent;}");
    auto *listLayout = new QVBoxLayout(content);
    listLayout->setContentsMargins(0, 0, 12, 0);
    listLayout->setSpacing(24);

    QList<NotificationRecord> records;
    const bool loaded = Database::notifications(records);
    if (!loaded || records.isEmpty()) {
        auto *card = new QFrame(content);
        card->setObjectName("notificationListState");
        card->setStyleSheet(
            "QFrame#notificationListState{background:#FFFDF8;border:1px solid #DBD1BA;"
            "border-radius:8px;}");
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(30, 24, 30, 24);
        layout->setSpacing(12);
        layout->addWidget(notificationText(
            loaded ? tr("No notifications yet") : tr("Unable to load notifications"),
            card, titleStyle));
        layout->addWidget(notificationText(
            loaded ? (AppSession::isStudent()
                          ? tr("Updates to your medical applications will appear here.")
                          : tr("Medical application updates will appear here."))
                   : (Database::error().isEmpty()
                          ? tr("Please refresh to try again.") : Database::error()),
            card, bodyStyle));
        listLayout->addWidget(card);
    } else {
        for (const auto &record : records) {
            auto *card = new QFrame(content);
            card->setObjectName(QStringLiteral("databaseNotificationCard%1").arg(record.id));
            card->setStyleSheet(
                QStringLiteral("QFrame#%1{background:#FFFDF8;border:1px solid #DBD1BA;"
                               "border-radius:8px;}").arg(card->objectName()));
            auto *cardLayout = new QHBoxLayout(card);
            cardLayout->setContentsMargins(20, 18, 24, 18);
            cardLayout->setSpacing(24);

            const QString color = record.type == "APPROVAL"
                ? QStringLiteral("#3F6449")
                : record.type == "DECLINE"
                    ? QStringLiteral("#7A263A") : QStringLiteral("#D6A21E");
            auto *accent = new QFrame(card);
            accent->setFixedWidth(10);
            accent->setMinimumHeight(50);
            accent->setStyleSheet(
                QStringLiteral("background:%1;border:none;border-radius:5px;").arg(color));
            cardLayout->addWidget(accent);

            auto *textLayout = new QVBoxLayout;
            textLayout->setContentsMargins(0, 0, 0, 0);
            textLayout->setSpacing(12);
            auto *top = new QHBoxLayout;
            top->setContentsMargins(0, 0, 0, 0);
            top->setSpacing(18);
            auto *title = notificationText(record.title, card, titleStyle);
            title->setObjectName(QStringLiteral("notificationTitle%1").arg(record.id));
            top->addWidget(title, 1);
            if (record.createdAt.isValid()) {
                auto *time = notificationText(
                    record.createdAt.toLocalTime().toString("dd MMM yyyy, hh:mm"), card, mutedStyle);
                time->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                top->addWidget(time);
            }
            textLayout->addLayout(top);
            auto *message = notificationText(record.message, card, bodyStyle);
            message->setObjectName(QStringLiteral("notificationMessage%1").arg(record.id));
            textLayout->addWidget(message);
            if (!record.isRead) {
                auto *readButton = new QPushButton(tr("Mark as read"), card);
                readButton->setObjectName(QStringLiteral("readNotification%1").arg(record.id));
                readButton->setAutoDefault(false);
                readButton->setCursor(Qt::PointingHandCursor);
                readButton->setStyleSheet(
                    "QPushButton{font-family:Inter;font-size:11px;color:#7A263A;"
                    "background:transparent;border:none;text-align:left;padding:3px 0px;}"
                    "QPushButton:hover{text-decoration:underline;}");
                connect(readButton, &QPushButton::clicked, this, [this, id = record.id]() {
                    if (!AppSession::authenticated()) return;
                    if (!Database::markNotificationRead(id)) {
                        QMessageBox::warning(
                            this, tr("Notification"),
                            Database::error().isEmpty()
                                ? tr("Unable to mark this notification as read.") : Database::error());
                        return;
                    }
                    refreshData();
                });
                textLayout->addWidget(readButton, 0, Qt::AlignLeft);
            }
            cardLayout->addLayout(textLayout, 1);
            listLayout->addWidget(card);
        }
    }
    listLayout->addStretch();

    QWidget *previous = notificationList->takeWidget();
    notificationList->setWidget(content);
    if (previous) previous->deleteLater();
}

void Notifications::onBackClicked()
{
    close();
}
