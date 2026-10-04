#include "umams.h"
#include "ui_umams.h"
#include "studentlogin.h"
#include "dashboard.h"
#include "adminlogin.h"

UMAMS::UMAMS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UMAMS)
{
    ui->setupUi(this);

    // Student Login
    connect(ui->btnStudentLogin,
            &QPushButton::clicked,
            this,
            &UMAMS::onStudentLoginClicked);

    // Admin Login
    connect(ui->btnAdminLogin,
            &QPushButton::clicked,
            this,
            &UMAMS::onAdminLoginClicked);
}

UMAMS::~UMAMS()
{
    delete ui;
}


// ==========================================
// STUDENT LOGIN
// ==========================================

void UMAMS::onStudentLoginClicked()
{
    StudentLogin login(this);

    login.exec();
}


// ==========================================
// ADMIN LOGIN
// ==========================================

void UMAMS::onAdminLoginClicked()
{
    AdminLogin login(this);
    login.exec();
}