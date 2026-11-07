#ifndef DIALOG_H
#define DIALOG_H
#include <QDialog>
namespace Ui { class Dialog; }
class Dialog : public QDialog {
    Q_OBJECT
public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
public slots:
    void refreshData();
private slots:
    void onSearchClicked();
    void onStatusChanged(int index);
    void onApproveClicked();
    void onDeclineClicked();
    void onViewDetailsClicked();
    void onBackClicked();
private:
    bool loadData(bool reportError);
    int selectedApplicationId() const;
    void reviewSelected(const QString &status);
    Ui::Dialog *ui;
};
#endif
