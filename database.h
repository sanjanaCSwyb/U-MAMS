#ifndef DATABASE_H
#define DATABASE_H

#include "app_session.h"
#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QString>

struct CourseRecord { QString name; QString code; };
struct ApplicationRecord {
    int id = 0;
    QString studentId, studentName, medicalId, reason, status;
    QDate fromDate, toDate;
    QDateTime submittedAt, reviewedAt;
    QList<CourseRecord> courses;
};
struct NotificationRecord {
    int id = 0;
    QString title, message, type;
    QDateTime createdAt;
    bool isRead = false;
};
struct ReportSummary { int total=0, pending=0, approved=0, declined=0; };

class Database {
public:
    static bool connect();
    static void close();
    static QString error();
    static QString configPath();
    static QSqlDatabase connection();
    static bool authenticate(const QString &login, const QString &password,
                             AppSession::Role expectedRole, AppSession::Identity &identity);
    static bool registerStudent(const QString &name, const QString &email,
                                const QString &registrationNumber, const QString &department,
                                const QString &phoneNumber, const QString &password);
    static bool profile(AppSession::Identity &identity);
    static bool studentProfile(const QString &studentId, AppSession::Identity &identity);
    static bool changePassword(const QString &currentPassword, const QString &newPassword);
    static bool applications(QList<ApplicationRecord> &result,
                             const QString &studentFilter = {}, const QString &status = {});
    static bool monthlyApprovedApplications(const QDate &month, QList<ApplicationRecord> &result);
    static QString registrationCategory(const QString &studentId);
    static bool application(int id, ApplicationRecord &result);
    static bool submitApplication(const QString &studentId, const QString &reason,
                                  const QDate &from, const QDate &to,
                                  const QList<CourseRecord> &courses, int &newId,
                                  QString *newMedicalId = nullptr);
    static bool reviewApplication(int id, const QString &newStatus);
    static bool nextPendingApplication(ApplicationRecord &result);
    static bool notifications(QList<NotificationRecord> &result);
    static bool markNotificationRead(int id);
    static bool summary(ReportSummary &result);
    static QString applicationCode(int id);
    static QString medicalCode(const QString &studentId, const QDate &submissionDate, qint64 dailyNumber);
    static int applicationId(const QString &code);
};

#endif // DATABASE_H
