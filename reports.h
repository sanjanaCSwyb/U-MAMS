#ifndef REPORTS_H
#define REPORTS_H
#include <QDialog>
namespace Ui { class Reports; }
class Reports : public QDialog {
    Q_OBJECT
public:
    explicit Reports(QWidget *parent = nullptr);
    ~Reports();
public slots:
    void refreshData();
private slots:
    void onExportReportClicked();
    void onBackClicked();
private:
    Ui::Reports *ui;
};
#endif
