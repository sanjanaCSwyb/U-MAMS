#include "umams.h"
#include "ui_umams.h"
#include "figma_ui.h"

#include "studentlogin.h"
#include "adminlogin.h"
#include "studentdashboard.h"
#include "dashboard.h"

#include <QApplication>
#include <QPushButton>
#include <QScopedValueRollback>

UMAMS::UMAMS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::UMAMS)
{
    ui->setupUi(this);
    setWindowTitle("U-MAMS");
    ui->menubar->hide();
    ui->statusbar->hide();
    // Keep the portal card layout defined in the .ui file.
    connect(ui->btnStudentLogin, &QPushButton::clicked,
            this, &UMAMS::onStudentLoginClicked);
    connect(ui->btnAdminLogin, &QPushButton::clicked,
            this, &UMAMS::onAdminLoginClicked);
    FigmaUi::install(this);
}

UMAMS::~UMAMS()
{
    delete ui;
}


void UMAMS::onStudentLoginClicked()
{
    runLoginFlow(AppSession::Role::Student);
}

void UMAMS::onAdminLoginClicked()
{
    runLoginFlow(AppSession::Role::Admin);
}

void UMAMS::runLoginFlow(AppSession::Role role)
{
    if (loginFlowActive) return;
    QScopedValueRollback<bool> running(loginFlowActive, true);
    const bool quitWhenClosed = QApplication::quitOnLastWindowClosed();
    // Completing one modal window must not quit the event loop before its
    // replacement is shown. The portal decides when the entire flow ends.
    QApplication::setQuitOnLastWindowClosed(false);
    AppSession::clear();
    hide();

    bool returnToPortal = false;
    for (;;) {
        AppSession::Identity user;
        int result = QDialog::Rejected;
        // Destroy the completed login before opening a parentless dashboard.
        // Windows must never reactivate a hidden login that owns the dashboard.
        if (role == AppSession::Role::Student) {
            StudentLogin login;
            result = login.exec();
            user = login.authenticatedIdentity();
        } else {
            AdminLogin login;
            result = login.exec();
            user = login.authenticatedIdentity();
        }
        if (result == AppSession::LoginBackToPortal) {
            returnToPortal = true;
            break;
        }
        if (result != AppSession::LoginAuthenticated || user.role != role || user.userId <= 0)
            break;

        bool requestedLogout = false;
        {
            AppSession::LoginSession session(user);
            if (role == AppSession::Role::Student) {
                StudentDashboard dashboard;
                dashboard.exec();
            } else {
                Dashboard dashboard;
                dashboard.exec();
            }
            requestedLogout = AppSession::logoutRequested();
        }
        if (!requestedLogout) break;
    }

    AppSession::clear();
    if (returnToPortal) {
        show();
        raise();
        activateWindow();
    } else {
        close();
    }
    QApplication::setQuitOnLastWindowClosed(quitWhenClosed);
    if (!returnToPortal && quitWhenClosed) QApplication::quit();
}
