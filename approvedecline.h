#ifndef APPROVEDECLINE_H
#define APPROVEDECLINE_H
#include <QDialog>
namespace Ui { class ApproveDecline; }
class ApproveDecline : public QDialog {
    Q_OBJECT
public:
    explicit ApproveDecline(QWidget *parent = nullptr);
    ~ApproveDecline();
public slots:
    void refreshData();
private slots:
    void onApproveClicked();
    void onDeclineClicked();
    void onBackClicked();
private:
    bool loadData(bool reportError);
    void reviewCurrent(const QString &status);
    int currentApplicationId = 0;
    Ui::ApproveDecline *ui;
};
#endif
