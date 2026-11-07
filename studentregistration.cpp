#include "studentregistration.h"
#include "ui_studentregistration.h"
#include "database.h"
#include "figma_ui.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QScopedValueRollback>

StudentRegistration::StudentRegistration(QWidget *parent)
    : QDialog(parent), ui(new Ui::StudentRegistration)
{
    ui->setupUi(this);
    connect(ui->btnRegister, &QPushButton::clicked,
            this, &StudentRegistration::onRegisterClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->chkShowPasswords, &QCheckBox::toggled, this, [this](bool visible) {
        const auto mode = visible ? QLineEdit::Normal : QLineEdit::Password;
        ui->txtPassword->setEchoMode(mode);
        ui->txtConfirmPassword->setEchoMode(mode);
    });
    FigmaUi::install(this);
}

StudentRegistration::~StudentRegistration()
{
    delete ui;
}

void StudentRegistration::onRegisterClicked()
{
    if (submissionInProgress) return;
    QScopedValueRollback<bool> submitting(submissionInProgress, true);

    const QString name = ui->txtNameWithInitials->text().trimmed();
    const QString email = ui->txtStudentEmail->text().trimmed();
    const QString registrationNumber = ui->txtRegistrationNumber->text().trimmed();
    const QString department = ui->txtDepartment->text().trimmed();
    const QString phoneNumber = ui->txtPhoneNumber->text().trimmed();
    const QString password = ui->txtPassword->text();
    const QString confirmation = ui->txtConfirmPassword->text();

    auto invalid = [this](const QString &message, QLineEdit *field) {
        QMessageBox::warning(this, tr("Check Registration Details"), message);
        field->setFocus();
    };
    const QList<QPair<QLineEdit *, QString>> requiredFields = {
        {ui->txtNameWithInitials, name}, {ui->txtStudentEmail, email},
        {ui->txtRegistrationNumber, registrationNumber}, {ui->txtDepartment, department},
        {ui->txtPhoneNumber, phoneNumber},
        {ui->txtPassword, password}, {ui->txtConfirmPassword, confirmation}
    };
    for (const auto &field : requiredFields) {
        if (field.second.isEmpty()) {
            invalid(tr("Please complete all seven fields."), field.first);
            return;
        }
    }
    if (name.size() > 150) {
        invalid(tr("Use a name containing no more than 150 characters."), ui->txtNameWithInitials);
        return;
    }
    if (email.size() > 150) {
        invalid(tr("Use a student email containing no more than 150 characters."), ui->txtStudentEmail);
        return;
    }
    static const QRegularExpression registrationPattern(QStringLiteral("^[A-Za-z0-9_/-]{1,20}$"));
    if (!registrationPattern.match(registrationNumber).hasMatch()) {
        invalid(tr("Use a registration number of up to 20 letters or numbers. Hyphens, underscores and slashes are allowed."),
                ui->txtRegistrationNumber);
        return;
    }
    if (department.size() > 150) {
        invalid(tr("Use a department containing no more than 150 characters."), ui->txtDepartment);
        return;
    }
    QString normalizedPhone = phoneNumber;
    normalizedPhone.remove(' ').remove('(').remove(')').remove('-');
    static const QRegularExpression phoneCharacters(QStringLiteral("^\\+?[0-9() -]+$"));
    static const QRegularExpression phoneDigits(QStringLiteral("^\\+?[0-9]{7,15}$"));
    if (phoneNumber.size() > 32 || !phoneCharacters.match(phoneNumber).hasMatch() ||
        !phoneDigits.match(normalizedPhone).hasMatch()) {
        invalid(tr("Enter a phone number with 7 to 15 digits. You may use a leading +, spaces, parentheses or hyphens (up to 32 characters)."),
                ui->txtPhoneNumber);
        return;
    }
    if (password.size() < 8 || password.size() > 128) {
        invalid(tr("Choose a password containing 8 to 128 characters."), ui->txtPassword);
        return;
    }
    if (password != confirmation) {
        invalid(tr("The passwords do not match. Please enter the same password in both fields."), ui->txtConfirmPassword);
        return;
    }

    ui->btnRegister->setEnabled(false);
    const bool saved = Database::registerStudent(name, email, registrationNumber, department, phoneNumber, password);
    ui->btnRegister->setEnabled(true);
    if (!saved) {
        QMessageBox::warning(this, tr("Registration Failed"),
                             Database::error().isEmpty()
                                 ? tr("Your account could not be created. Please try again.")
                                 : Database::error());
        return;
    }

    createdRegistrationNumber = registrationNumber;
    ui->txtPassword->clear();
    ui->txtConfirmPassword->clear();
    QMessageBox::information(this, tr("Registration Complete"),
                             tr("Your student account has been created.\nSign in using your registration number and password."));
    accept();
}
