#include "dashboard.h"
#include "ui_dashboard.h"
#include "medicalapplication.h"

Dashboard::Dashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dashboard)
{
    ui->setupUi(this);

    connect(ui->btnMedicalApplications, &QPushButton::clicked,
            this, &Dashboard::onMedicalApplicationClicked);
}

Dashboard::~Dashboard()
{
    delete ui;
}

void Dashboard::onMedicalApplicationClicked()
{
    MedicalApplication medicalApplication(this);
    medicalApplication.exec();
}