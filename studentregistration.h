#ifndef STUDENTREGISTRATION_H
#define STUDENTREGISTRATION_H

#include <QDialog>
#include <QString>

namespace Ui { class StudentRegistration; }

class StudentRegistration final : public QDialog
{
    Q_OBJECT

public:
    explicit StudentRegistration(QWidget *parent = nullptr);
    ~StudentRegistration() override;

    QString registeredNumber() const { return createdRegistrationNumber; }

private slots:
    void onRegisterClicked();

private:
    Ui::StudentRegistration *ui;
    QString createdRegistrationNumber;
    bool submissionInProgress = false;
};

#endif // STUDENTREGISTRATION_H
