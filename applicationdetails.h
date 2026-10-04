#ifndef APPLICATIONDETAILS_H
#define APPLICATIONDETAILS_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QPair>

namespace Ui {
class ApplicationDetails;
}

class ApplicationDetails : public QDialog
{
    Q_OBJECT

public:
    explicit ApplicationDetails(QWidget *parent = nullptr);
    ~ApplicationDetails();

    void setApplicationData(
        const QString &applicationID,
        const QString &studentID,
        const QString &studentName,
        const QString &medicalID,
        const QString &reason,
        const QString &fromDate,
        const QString &toDate,
        const QString &status,
        const QList<QPair<QString, QString>> &courses
        );

signals:
    void statusChanged(const QString &applicationID,
                       const QString &newStatus);

private slots:
    void onBackClicked();
    void onApproveClicked();

private:
    Ui::ApplicationDetails *ui;
    QString currentApplicationID;
};

#endif // APPLICATIONDETAILS_H