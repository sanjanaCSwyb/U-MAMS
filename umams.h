#ifndef UMAMS_H
#define UMAMS_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class UMAMS;
}
QT_END_NAMESPACE

class UMAMS : public QMainWindow
{
    Q_OBJECT

public:
    explicit UMAMS(QWidget *parent = nullptr);
    ~UMAMS() override;

private slots:
    void onStudentLoginClicked();
    void onAdminLoginClicked();

private:
    Ui::UMAMS *ui;
};

#endif // UMAMS_H