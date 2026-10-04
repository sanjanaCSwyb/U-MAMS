#include "notifications.h"
#include "ui_notifications.h"

#include <QPushButton>


Notifications::Notifications(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Notifications)
{
    ui->setupUi(this);

    setWindowTitle("U-MAMS - Notifications");


    // =====================================================
    // NOTIFICATION 1
    // =====================================================

    ui->lblNotificationTitle->setText(
        "Medical application submitted"
        );

    ui->lblNotificationMessage->setText(
        "APP-001 was submitted successfully."
        );

    ui->lblNotificationTime->setText(
        "Just now"
        );


    // =====================================================
    // NOTIFICATION 2
    // =====================================================

    ui->lblNotificationTitle2->setText(
        "Application approved"
        );

    ui->lblNotificationMessage2->setText(
        "APP-002 for student 249172 has been approved."
        );

    ui->lblNotificationTime2->setText(
        "Today"
        );


    // =====================================================
    // NOTIFICATION 3
    // =====================================================

    ui->lblNotificationTitle3->setText(
        "Application declined"
        );

    ui->lblNotificationMessage3->setText(
        "APP-003 has been declined."
        );

    ui->lblNotificationTime3->setText(
        "Yesterday"
        );


    // =====================================================
    // BACK BUTTON
    // =====================================================

    connect(
        ui->btnBack,
        &QPushButton::clicked,
        this,
        &Notifications::onBackClicked
        );
}


// =========================================================
// DESTRUCTOR
// =========================================================

Notifications::~Notifications()
{
    delete ui;
}


// =========================================================
// BACK BUTTON
// =========================================================

void Notifications::onBackClicked()
{
    close();
}