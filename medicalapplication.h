#ifndef MEDICALAPPLICATION_H
#define MEDICALAPPLICATION_H

#include <QDialog>

namespace Ui {
class MedicalApplication;
}

class MedicalApplication : public QDialog
{
    Q_OBJECT

public:
    explicit MedicalApplication(QWidget *parent = nullptr);
    ~MedicalApplication();

private slots:
    void onAddCourseClicked();
    void onSubmitApplicationClicked();

private:
    Ui::MedicalApplication *ui;
};

#endif // MEDICALAPPLICATION_H