#ifndef APPLICATIONDETAILS_H
#define APPLICATIONDETAILS_H
#include <QDialog>
#include <QString>
#include <QList>
#include <QPair>
namespace Ui { class ApplicationDetails; }
class ApplicationDetails : public QDialog {
    Q_OBJECT
public:
    explicit ApplicationDetails(QWidget *parent = nullptr);
    ~ApplicationDetails();
    bool setApplicationId(int id);
    void setApplicationData(const QString &applicationID, const QString &studentID,
        const QString &studentName, const QString &medicalID, const QString &reason,
        const QString &fromDate, const QString &toDate, const QString &status,
        const QList<QPair<QString, QString>> &courses);
public slots:
    void refreshData();
signals:
    void statusChanged(const QString &applicationID, const QString &newStatus);
private slots:
    void onBackClicked();
    void onApproveClicked();
private:
    bool loadData(bool reportError);
    void clearData();
    Ui::ApplicationDetails *ui;
    QString currentApplicationID;
    int currentNumericId = 0;
};
#endif
