#include "database.h"
#include "password_hash.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>
#include <QSet>
#include <QTimeZone>
#include <limits>

namespace {
const QString connectionName="U_MAMS_CONNECTION";
QString lastError;
bool fail(const QString &message) { lastError=message; return false; }
bool execute(QSqlQuery &query,const QString &sql,const QVariantMap &values={})
{
    if(!query.prepare(sql)) return fail(query.lastError().text());
    for(auto it=values.cbegin();it!=values.cend();++it) query.bindValue(':'+it.key(),it.value());
    return query.exec() || fail(query.lastError().text());
}
QSqlDatabase db() { return QSqlDatabase::database(connectionName,false); }
QString roleName(AppSession::Role role) { return role==AppSession::Role::Admin ? "ADMIN" : "STUDENT"; }
const QString identitySelect="SELECT u.user_id,u.username,u.role,COALESCE(s.student_id,''),COALESCE(s.student_name,u.full_name),COALESCE(s.email,u.email),COALESCE(s.department,'') FROM users u LEFT JOIN students s ON s.user_id=u.user_id ";
AppSession::Identity readIdentity(const QSqlQuery &query)
{
    AppSession::Identity user;
    user.userId=query.value(0).toInt();user.username=query.value(1).toString();
    user.role=query.value(2).toString()=="ADMIN" ? AppSession::Role::Admin : AppSession::Role::Student;
    user.studentId=query.value(3).toString();user.fullName=query.value(4).toString();
    user.email=query.value(5).toString();user.department=query.value(6).toString();
    return user;
}
bool currentUser(AppSession::Identity &user)
{
    if(!AppSession::authenticated() || AppSession::identity().userId<=0) return fail("Please sign in to continue.");
    if(!Database::connect()) return false;
    QSqlQuery query(db());
    if(!execute(query,identitySelect+"WHERE u.user_id=:id AND u.is_active=1",{{"id",AppSession::identity().userId}})) return false;
    if(!query.next()) return fail("Your account is unavailable. Please sign in again.");
    user=readIdentity(query);
    const QString principal=user.role==AppSession::Role::Student ? user.studentId : user.username;
    if(user.role!=AppSession::currentRole() || principal!=AppSession::principal() || principal.isEmpty())
        return fail("Your session is no longer valid. Please sign in again.");
    return true;
}
bool administrator(AppSession::Identity &user)
{
    return currentUser(user) && (user.role==AppSession::Role::Admin || fail("Administrator access is required."));
}
class Transaction {
    QSqlDatabase connection;
    bool active=false;
public:
    Transaction():connection(db()),active(connection.transaction()) {
        if(!active) fail(connection.lastError().text());
    }
    ~Transaction() { if(active) connection.rollback(); }
    bool valid() const { return active; }
    bool commit() {
        if(!active) return false;
        if(!connection.commit()) return fail(connection.lastError().text());
        active=false;return true;
    }
};
QString titleStatus(QString status)
{
    status=status.toLower();if(!status.isEmpty()) status[0]=status[0].toUpper();return status;
}
const QString applicationSelect="SELECT a.application_id,a.student_id,s.student_name,a.medical_id,a.reason_of_absence,a.medical_from,a.medical_to,a.status,a.submitted_at,a.reviewed_at FROM medical_applications a JOIN students s ON s.student_id=a.student_id ";
ApplicationRecord readApplication(const QSqlQuery &query)
{
    ApplicationRecord result;
    result.id=query.value(0).toInt();result.studentId=query.value(1).toString();
    result.studentName=query.value(2).toString();result.medicalId=query.value(3).toString();
    result.reason=query.value(4).toString();result.fromDate=query.value(5).toDate();
    result.toDate=query.value(6).toDate();result.status=titleStatus(query.value(7).toString());
    const QTimeZone displayZone("Asia/Colombo");
    result.submittedAt=query.value(8).toDateTime().toTimeZone(displayZone);
    result.reviewedAt=query.value(9).toDateTime().toTimeZone(displayZone);return result;
}
bool addNotification(int userId,int applicationId,const QString &title,const QString &message,const QString &type)
{
    QSqlQuery query(db());
    return execute(query,"INSERT INTO notifications(user_id,application_id,title,message,notification_type) VALUES(:user,:application,:title,:message,:type)",
                   {{"user",userId},{"application",applicationId},{"title",title},{"message",message},{"type",type}});
}
}

QString Database::error() { return lastError; }
QString Database::configPath()
{
    if(qEnvironmentVariableIsSet("UMAMS_DB_CONFIG")) return qEnvironmentVariable("UMAMS_DB_CONFIG");
    const QString relativePath="Wayamba/U-MAMS/database.ini";
    const QString standardPath=QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)).filePath(relativePath);
#ifdef Q_OS_WIN
    const QString localRoot=qEnvironmentVariable("LOCALAPPDATA");
    if(!QFileInfo::exists(standardPath) && !localRoot.isEmpty()) {
        const QString localPath=QDir(localRoot).filePath(relativePath);
        if(QDir::cleanPath(localPath).compare(QDir::cleanPath(standardPath),Qt::CaseInsensitive)!=0 && QFileInfo::exists(localPath))
            return localPath;
    }
#endif
    return standardPath;
}
QSqlDatabase Database::connection() { connect();return db(); }
bool Database::connect()
{
    lastError.clear();
    if(QSqlDatabase::contains(connectionName) && db().isOpen()) {
        QSqlQuery ping(db());
        if(ping.exec("SELECT 1")) return true;
        db().close();
    }
    const QString settingsPath=configPath();
    auto configurationError=[&](const QString &message) {
        return fail(message+"\nDatabase configuration: "+(settingsPath.isEmpty() ? "(empty UMAMS_DB_CONFIG)" : QDir::toNativeSeparators(QFileInfo(settingsPath).absoluteFilePath())));
    };
    if(settingsPath.trimmed().isEmpty())
        return configurationError("UMAMS_DB_CONFIG is set but empty. Remove the override or select a database.ini file.");
    const bool environmentCredentials=qEnvironmentVariableIsSet("UMAMS_DB_NAME") && qEnvironmentVariableIsSet("UMAMS_DB_USER") && qEnvironmentVariableIsSet("UMAMS_DB_PASSWORD");
    const QFileInfo settingsFile(settingsPath);
    if(settingsFile.exists()) {
        QFile readableSettings(settingsPath);
        if(!settingsFile.isFile() || !readableSettings.open(QIODevice::ReadOnly))
            return configurationError("The database configuration file cannot be read. Check its file permissions.");
    } else if(!environmentCredentials) {
        return configurationError("The database configuration file was not found. Restore database.ini or supply the database environment settings.");
    }
    QSettings settings(settingsPath,QSettings::IniFormat);
    settings.beginGroup("mysql");
    settings.allKeys(); // Force parsing so malformed INI files are reported before connecting.
    if(settings.status()!=QSettings::NoError)
        return configurationError("The database configuration file is unreadable or has invalid INI syntax.");
    auto value=[&](const char *environment,const QString &key,const QString &fallback={}) {
        return qEnvironmentVariableIsSet(environment) ? qEnvironmentVariable(environment) : settings.value(key,fallback).toString();
    };
    const QString host=value("UMAMS_DB_HOST","host","127.0.0.1");
    const QString portText=value("UMAMS_DB_PORT","port","3306");
    const QString database=value("UMAMS_DB_NAME","database");
    const QString user=value("UMAMS_DB_USER","user");
    const QString password=value("UMAMS_DB_PASSWORD","password");
    if(qEnvironmentVariableIsSet("UMAMS_DB_PASSWORD") && password.isEmpty())
        return configurationError("UMAMS_DB_PASSWORD is set but empty. Remove the empty override or provide the database account password.");
    if(database.trimmed().isEmpty() || user.trimmed().isEmpty() || password.isEmpty())
        return configurationError("Database settings are incomplete. Set database, user and password in the [mysql] section or their UMAMS_DB_* environment overrides.");
    bool validPort=false;
    const int port=portText.toInt(&validPort);
    if(host.trimmed().isEmpty() || !validPort || port<1 || port>65535)
        return configurationError("Database host or port is invalid. Supply a host and a port between 1 and 65535.");
    if(!QSqlDatabase::isDriverAvailable("QMYSQL"))
        return fail("The Qt MySQL driver could not load. Check the MySQL client runtime beside the application.");
    QSqlDatabase connection=QSqlDatabase::contains(connectionName) ? db() : QSqlDatabase::addDatabase("QMYSQL",connectionName);
    connection.setHostName(host);
    connection.setPort(port);
    connection.setDatabaseName(database);
    connection.setUserName(user);
    connection.setPassword(password);
    connection.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=5;MYSQL_OPT_READ_TIMEOUT=10;MYSQL_OPT_WRITE_TIMEOUT=10");
    if(!connection.open()) {
        const QString code=connection.lastError().nativeErrorCode();
        return configurationError("Could not connect to MySQL. Check the server and configured database account."+(code.isEmpty() ? QString() : "\nMySQL error code: "+code));
    }
    QSqlQuery query(connection);
    if(!query.exec("SET time_zone = '+00:00'")) { connection.close();return fail(query.lastError().text()); }
    return true;
}
void Database::close()
{
    if(QSqlDatabase::contains(connectionName)) db().close();
}
bool Database::authenticate(const QString &login,const QString &password,AppSession::Role expectedRole,AppSession::Identity &identity)
{
    identity={};
    if(expectedRole==AppSession::Role::None || login.trimmed().isEmpty() || password.isEmpty()) return fail("Enter your username/student ID and password.");
    if(!connect()) return false;
    QSqlQuery query(db());
    const QString sql=identitySelect+"WHERE u.is_active=1 AND u.role=:role AND (u.username=:username OR s.student_id=:student) ORDER BY CASE WHEN s.student_id=:preferred THEN 0 ELSE 1 END,u.user_id LIMIT 1";
    if(!execute(query,sql,{{"role",roleName(expectedRole)},{"username",login.trimmed()},{"student",login.trimmed()},{"preferred",login.trimmed()}})) return false;
    if(!query.next()) return fail("Invalid username/student ID or password.");
    const auto user=readIdentity(query);
    QSqlQuery credential(db());
    if(!execute(credential,"SELECT password_hash FROM users WHERE user_id=:id",{{"id",user.userId}})) return false;
    if(!credential.next()) return fail("Invalid username/student ID or password.");
    const QString stored=credential.value(0).toString();
    if(!PasswordHash::verify(password,stored) || (user.role==AppSession::Role::Student && user.studentId.isEmpty()))
        return fail("Invalid username/student ID or password.");
    if(PasswordHash::needsUpgrade(stored)) {
        QSqlQuery upgrade(db());
        if(!execute(upgrade,"UPDATE users SET password_hash=:hash WHERE user_id=:id AND password_hash=:old",{{"hash",PasswordHash::encode(password)},{"id",user.userId},{"old",stored}})) return false;
        if(upgrade.numRowsAffected()!=1) return fail("The account changed. Please sign in again.");
    }
    identity=user;return true;
}
bool Database::registerStudent(const QString &name,const QString &email,const QString &registrationNumber,
                               const QString &department,const QString &phoneNumber,const QString &password)
{
    lastError.clear();
    if(AppSession::authenticated()) return fail("Sign out before creating a student account.");
    const QString fullName=name.trimmed(),address=email.trimmed().toLower();
    const QString studentId=registrationNumber.trimmed(),unit=department.trimmed();
    auto plainText=[](const QString &value) {
        for(const QChar character:value) if(character.category()==QChar::Other_Control) return false;
        return true;
    };
    if(fullName.isEmpty() || fullName.size()>150 || !plainText(fullName))
        return fail("Enter your name with initials using 1 to 150 characters.");
    if(unit.isEmpty() || unit.size()>150 || !plainText(unit))
        return fail("Enter a department using 1 to 150 characters.");
    static const QRegularExpression registration("^[A-Za-z0-9_/-]{1,20}$");
    if(!registration.match(studentId).hasMatch())
        return fail("Use a registration number of 1 to 20 letters, numbers, slashes, underscores or hyphens.");
    static const QRegularExpression mailbox(
        "^[a-z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-z0-9](?:[a-z0-9-]*[a-z0-9])?(?:\\.[a-z0-9](?:[a-z0-9-]*[a-z0-9])?)+$");
    const QString local=address.section('@',0,0);
    bool domainLabelsValid=true;
    for(const QString &label:address.section('@',-1).split('.'))
        if(label.size()>63) domainLabelsValid=false;
    if(address.size()>150 || local.size()>64 || local.startsWith('.') || local.endsWith('.') || local.contains("..") ||
       !domainLabelsValid || !mailbox.match(address).hasMatch())
        return fail("Enter a valid student email address using no more than 150 characters.");
    const QString enteredPhone=phoneNumber.trimmed();
    static const QRegularExpression phoneFormat("^\\+?[0-9() -]+$");
    static const QRegularExpression phoneSeparators("[() -]");
    static const QRegularExpression phoneDigits("^\\+?[0-9]{7,15}$");
    QString phone=enteredPhone;
    phone.remove(phoneSeparators);
    if(enteredPhone.size()>32 || !phoneFormat.match(enteredPhone).hasMatch() || !phoneDigits.match(phone).hasMatch())
        return fail("Enter a phone number with 7 to 15 digits. You may use a leading +, spaces, parentheses or hyphens.");
    if(password.size()<8 || password.size()>128)
        return fail("Use a password containing 8 to 128 characters.");
    if(!connect()) return false;
    QSqlQuery query(db());
    if(!query.prepare("CALL umams_register_student(:name,:email,:student,:department,:phone,:hash)"))
        return fail("Student registration is unavailable. Please contact the administrator.");
    query.bindValue(":name",fullName);query.bindValue(":email",address);
    query.bindValue(":student",studentId);query.bindValue(":department",unit);
    query.bindValue(":phone",phone);
    query.bindValue(":hash",PasswordHash::encode(password));
    const auto registrationFailure=[&] {
        const QString message=query.lastError().text();
        const QString code=query.lastError().nativeErrorCode();
        if(message.contains("UMAMS_REG_ID_EXISTS") || message.contains("UMAMS_REG_USERNAME_EXISTS") || code=="1062")
            return fail("This registration number is already registered. Please sign in or contact the administrator.");
        if(message.contains("UMAMS_REG_EMAIL_EXISTS"))
            return fail("This email address is already registered. Please use your existing account or contact the administrator.");
        if(message.contains("UMAMS_REG_INVALID_INPUT"))
            return fail("Check your registration details and try again.");
        if(code=="1305" || code=="1370" || code=="1142")
            return fail("Student registration is unavailable. Please contact the administrator.");
        return fail("We could not create your account. Please try again.");
    };
    if(!query.exec()) return registrationFailure();
    // Drain the CALL status result before reusing the QMYSQL connection.
    while(query.nextResult()) {}
    if(query.lastError().isValid()) return registrationFailure();
    return true;
}
bool Database::profile(AppSession::Identity &identity) { lastError.clear();identity={};return currentUser(identity); }
bool Database::studentProfile(const QString &studentId,AppSession::Identity &identity)
{
    lastError.clear();identity={};AppSession::Identity actor;
    if(!currentUser(actor)) return false;
    if(actor.role==AppSession::Role::Student && actor.studentId!=studentId) return fail("You can only access your own student profile.");
    QSqlQuery query(db());
    if(!execute(query,identitySelect+"WHERE s.student_id=:student AND u.role='STUDENT' AND u.is_active=1",{{"student",studentId}})) return false;
    if(!query.next()) return fail("No active student account matches that Student ID.");
    identity=readIdentity(query);return true;
}
bool Database::changePassword(const QString &currentPassword,const QString &newPassword)
{
    lastError.clear();AppSession::Identity user;
    if(!currentUser(user)) return false;
    if(newPassword.size()<8 || newPassword.size()>128) return fail("Use a new password containing 8 to 128 characters.");
    if(newPassword==currentPassword) return fail("Choose a password different from the current password.");
    QSqlQuery query(db());
    if(!execute(query,"SELECT password_hash FROM users WHERE user_id=:id",{{"id",user.userId}}) || !query.next()) return false;
    const QString previous=query.value(0).toString();
    if(!PasswordHash::verify(currentPassword,previous)) return fail("The current password is incorrect.");
    if(!execute(query,"UPDATE users SET password_hash=:hash WHERE user_id=:id AND password_hash=:old AND is_active=1",{{"hash",PasswordHash::encode(newPassword)},{"id",user.userId},{"old",previous}})) return false;
    return query.numRowsAffected()==1 || fail("The account changed. Please sign in again.");
}
bool Database::applications(QList<ApplicationRecord> &result,const QString &studentFilter,const QString &status)
{
    lastError.clear();result.clear();AppSession::Identity user;
    if(!currentUser(user)) return false;
    QString sql=applicationSelect+"WHERE 1=1 ";QVariantMap values;
    if(user.role==AppSession::Role::Student) {
        if(!studentFilter.isEmpty() && studentFilter!=user.studentId) return fail("You can only view your own applications.");
        sql+="AND a.student_id=:student ";values["student"]=user.studentId;
    } else if(!studentFilter.trimmed().isEmpty()) {
        sql+="AND a.student_id LIKE :student ";values["student"]='%'+studentFilter.trimmed()+'%';
    }
    const QString selected=status.trimmed().toUpper();
    if(!selected.isEmpty() && selected!="ALL" && selected!="ALL STATUSES") {
        if(selected!="PENDING" && selected!="APPROVED" && selected!="DECLINED") return fail("Invalid application status filter.");
        sql+="AND a.status=:status ";values["status"]=selected;
    }
    sql+="ORDER BY a.submitted_at DESC,a.application_id DESC";
    QSqlQuery query(db());if(!execute(query,sql,values)) return false;
    while(query.next()) result.append(readApplication(query));return true;
}
bool Database::monthlyApprovedApplications(const QDate &month,QList<ApplicationRecord> &result)
{
    lastError.clear();result.clear();
    if(!month.isValid() || month.year()<1000 || month.year()>9999)
        return fail("Select a valid month between January 1000 and December 9999.");
    AppSession::Identity actor;
    if(!administrator(actor)) return false;
    const QDate firstDay(month.year(),month.month(),1);
    const QDate lastDay(month.year(),month.month(),firstDay.daysInMonth());
    QSqlQuery query(db());
    const QString sql=applicationSelect+
        "WHERE a.status='APPROVED' AND a.medical_from<=:end AND a.medical_to>=:start "
        "ORDER BY a.student_id,a.medical_from,a.medical_to,a.application_id";
    if(!execute(query,sql,{{"start",firstDay},{"end",lastDay}})) return false;
    while(query.next()) result.append(readApplication(query));
    if(query.lastError().isValid()) {result.clear();return fail(query.lastError().text());}
    return true;
}
QString Database::registrationCategory(const QString &studentId)
{
    if(studentId.size()<3) return QStringLiteral("Other");
    for(int i=0;i<3;++i) {
        const ushort character=studentId.at(i).unicode();
        if(character<'0' || character>'9') return QStringLiteral("Other");
    }
    return studentId.left(3);
}
bool Database::application(int id,ApplicationRecord &result)
{
    lastError.clear();result={};AppSession::Identity user;
    if(!currentUser(user)) return false;
    QSqlQuery query(db());QString sql=applicationSelect+"WHERE a.application_id=:id";QVariantMap values{{"id",id}};
    if(user.role==AppSession::Role::Student) {sql+=" AND a.student_id=:student";values["student"]=user.studentId;}
    if(!execute(query,sql,values)) return false;
    if(!query.next()) return fail("This application is unavailable or does not belong to your account.");
    result=readApplication(query);
    if(!execute(query,"SELECT course_name,course_code FROM missed_courses WHERE application_id=:id ORDER BY course_id",{{"id",id}})) return false;
    while(query.next()) result.courses.append({query.value(0).toString(),query.value(1).toString()});return true;
}
bool Database::submitApplication(const QString &studentId,const QString &reason,const QDate &from,const QDate &to,const QList<CourseRecord> &courses,int &newId,QString *newMedicalId)
{
    lastError.clear();newId=0;
    if(newMedicalId) newMedicalId->clear();
    AppSession::Identity actor;
    if(!currentUser(actor)) return false;
    if(actor.role!=AppSession::Role::Student) return fail("Only students can submit medical applications.");
    if(studentId!=actor.studentId) return fail("You can only submit your own medical application.");
    if(studentId.trimmed().isEmpty() || reason.trimmed().isEmpty() || reason.toUtf8().size()>60000)
        return fail("Enter a valid Student ID and reason for absence.");
    if(!from.isValid() || !to.isValid() || from>to) return fail("Select a valid medical period, with From on or before To.");
    if(courses.isEmpty() || courses.size()>50) return fail("Add between 1 and 50 missed courses.");
    QSet<QString> codes;
    for(const auto &course:courses) {
        if(course.name.trimmed().isEmpty() || course.code.trimmed().isEmpty() || course.name.size()>150 || course.code.size()>50) return fail("Each course needs a valid name and code.");
        const QString code=course.code.trimmed().toCaseFolded();
        if(codes.contains(code)) return fail("A course code has been added more than once.");codes.insert(code);
    }
    Transaction transaction;if(!transaction.valid()) return false;
    QSqlQuery query(db());
    // Capture the server clock once. This value supplies both the Sri Lankan
    // counter date and the record's submitted_at, irrespective of medical dates.
    if(!query.exec("SELECT UTC_TIMESTAMP()")) return fail(query.lastError().text());
    if(!query.next()) return fail("The database submission time is unavailable. Please try again.");
    const QDateTime returnedTime=query.value(0).toDateTime();
    const QTimeZone utcZone(QTimeZone::UTC),submissionZone("Asia/Colombo");
    const QDateTime submittedUtc(returnedTime.date(),returnedTime.time(),utcZone);
    if(!returnedTime.isValid() || !submittedUtc.isValid() || !submissionZone.isValid())
        return fail("The database submission time is invalid. Please contact the administrator.");
    const QDate submissionDate=submittedUtc.toTimeZone(submissionZone).date();
    const QDateTime dayStartUtc=QDateTime(submissionDate,QTime(0,0),submissionZone).toUTC();
    const QDateTime dayEndUtc=QDateTime(submissionDate.addDays(1),QTime(0,0),submissionZone).toUTC();

    // The upsert obtains an exclusive row lock even for an existing counter.
    // Keep the complete DATE in the key: MM-DD is only the display component.
    if(!execute(query,"INSERT INTO medical_submission_counters(student_id,submission_date,last_number) VALUES(:student,:date,0) "
                      "ON DUPLICATE KEY UPDATE last_number=last_number",{{"student",studentId},{"date",submissionDate}})) return false;
    if(!execute(query,"SELECT last_number FROM medical_submission_counters WHERE student_id=:student AND submission_date=:date FOR UPDATE",
                {{"student",studentId},{"date",submissionDate}})) return false;
    if(!query.next()) return fail("A daily Medical ID could not be reserved. Please try again.");
    bool validCounter=false;
    const qulonglong lockedNumber=query.value(0).toULongLong(&validCounter);
    const qulonglong maximumNumber=static_cast<qulonglong>(std::numeric_limits<qint64>::max());
    if(!validCounter || lockedNumber>=maximumNumber) return fail("The daily Medical ID limit has been reached. Please contact the administrator.");
    qint64 lastNumber=static_cast<qint64>(lockedNumber);
    if(lastNumber==0) {
        // This is the first consistent table read in this transaction, after
        // acquiring the counter lock. Avoid a stale pre-lock InnoDB snapshot.
        if(!execute(query,"SELECT medical_id FROM medical_applications WHERE student_id=:student AND submitted_at>=:start AND submitted_at<:end",
                    {{"student",studentId},{"start",dayStartUtc},{"end",dayEndUtc}})) return false;
        qint64 existingCount=0,largestSuffix=0;
        const QString prefix=studentId+'-'+submissionDate.toString("MM-dd")+'-';
        static const QRegularExpression digits("^[0-9]+$");
        while(query.next()) {
            if(existingCount==std::numeric_limits<qint64>::max()) return fail("The daily Medical ID limit has been reached. Please contact the administrator.");
            ++existingCount;
            const QString previous=query.value(0).toString();
            // Literal matching keeps %, _, regex characters and hyphens in a
            // Student ID from acting as a pattern or selecting another ID.
            if(!previous.startsWith(prefix,Qt::CaseSensitive)) continue;
            const QString suffix=previous.mid(prefix.size());
            if(!digits.match(suffix).hasMatch()) continue;
            bool validSuffix=false;
            const qulonglong value=suffix.toULongLong(&validSuffix);
            if(!validSuffix || value>=maximumNumber) return fail("Existing Medical IDs exceed the daily numbering limit. Please contact the administrator.");
            largestSuffix=qMax(largestSuffix,static_cast<qint64>(value));
        }
        lastNumber=qMax(existingCount,largestSuffix);
    }
    if(lastNumber>=std::numeric_limits<qint64>::max()) return fail("The daily Medical ID limit has been reached. Please contact the administrator.");
    const qint64 dailyNumber=lastNumber+1;
    const QString medicalId=medicalCode(studentId,submissionDate,dailyNumber);
    if(medicalId.isEmpty()) return fail("A daily Medical ID could not be assigned. Please try again.");
    if(!execute(query,"UPDATE medical_submission_counters SET last_number=:number WHERE student_id=:student AND submission_date=:date AND last_number=:previous",
                {{"number",dailyNumber},{"student",studentId},{"date",submissionDate},{"previous",lockedNumber}})) return false;
    if(query.numRowsAffected()!=1) return fail("A daily Medical ID could not be reserved. Please try again.");

    // Recheck the persisted student/account association after the lock; role or
    // account changes never allow a caller to submit for a different account.
    if(!execute(query,"SELECT s.user_id FROM students s JOIN users u ON u.user_id=s.user_id WHERE s.student_id=:student AND u.user_id=:user AND u.role='STUDENT' AND u.is_active=1",
                {{"student",studentId},{"user",actor.userId}})) return false;
    if(!query.next()) return fail("Your student account is unavailable. Please sign in again.");
    const int studentUser=query.value(0).toInt();
    if(!execute(query,"INSERT INTO medical_applications(student_id,medical_id,reason_of_absence,medical_from,medical_to,submitted_at) VALUES(:student,:medical,:reason,:from,:to,:submitted)",
                {{"student",studentId},{"medical",medicalId},{"reason",reason.trimmed()},{"from",from},{"to",to},{"submitted",submittedUtc}})) return false;
    const int id=query.lastInsertId().toInt();
    if(id<=0) return fail("The application could not be saved. Please try again.");
    for(const auto &course:courses)
        if(!execute(query,"INSERT INTO missed_courses(application_id,course_name,course_code) VALUES(:id,:name,:code)",{{"id",id},{"name",course.name.trimmed()},{"code",course.code.trimmed()}})) return false;
    const QString code=applicationCode(id);
    if(!addNotification(studentUser,id,"Medical application submitted",code+" was submitted successfully.","SUBMISSION")) return false;
    if(!execute(query,"INSERT INTO notifications(user_id,application_id,title,message,notification_type) SELECT user_id,:application,:title,:message,'SUBMISSION' FROM users WHERE role='ADMIN' AND is_active=1",{{"application",id},{"title","New medical application"},{"message",code+" was submitted by student "+studentId+"."}})) return false;
    // The counter, final Medical ID, application, courses and notifications are
    // one transaction. No value is returned if any write or the commit fails.
    if(!transaction.commit()) return false;
    newId=id;
    if(newMedicalId) *newMedicalId=medicalId;
    return true;
}

bool Database::reviewApplication(int id,const QString &newStatus)
{
    lastError.clear();AppSession::Identity actor;if(!administrator(actor)) return false;
    const QString status=newStatus.toUpper();
    if(status!="APPROVED" && status!="DECLINED") return fail("Choose Approve or Decline.");
    Transaction transaction;if(!transaction.valid()) return false;
    QSqlQuery query(db());
    if(!execute(query,"SELECT status,student_id FROM medical_applications WHERE application_id=:id FOR UPDATE",{{"id",id}})) return false;
    if(!query.next()) return fail("The selected application no longer exists.");
    if(query.value(0).toString()!="PENDING") return fail("This application has already been reviewed. Refresh the list.");
    const QString studentId=query.value(1).toString();
    if(!execute(query,"SELECT user_id FROM students WHERE student_id=:student",{{"student",studentId}}) || !query.next()) return fail("The student account is unavailable.");
    const int studentUser=query.value(0).toInt();
    if(!execute(query,"UPDATE medical_applications SET status=:status,reviewed_by=:reviewer,reviewed_at=CURRENT_TIMESTAMP WHERE application_id=:id AND status='PENDING'",{{"status",status},{"reviewer",actor.userId},{"id",id}})) return false;
    if(query.numRowsAffected()!=1) return fail("This application has already been reviewed.");
    const QString lower=status.toLower();const QString title="Application "+lower;
    if(!addNotification(studentUser,id,title,applicationCode(id)+" has been "+lower+".",status=="APPROVED" ? "APPROVAL" : "DECLINE")) return false;
    if(!execute(query,"INSERT INTO notifications(user_id,application_id,title,message,notification_type) SELECT user_id,:application,:title,:message,:type FROM users WHERE role='ADMIN' AND is_active=1",{{"application",id},{"title",title},{"message",applicationCode(id)+" was "+lower+" by "+actor.fullName+"."},{"type",status=="APPROVED" ? "APPROVAL" : "DECLINE"}})) return false;
    return transaction.commit();
}
bool Database::nextPendingApplication(ApplicationRecord &result)
{
    lastError.clear();result={};AppSession::Identity actor;if(!administrator(actor)) return false;
    QSqlQuery query(db());
    if(!query.exec("SELECT application_id FROM medical_applications WHERE status='PENDING' ORDER BY submitted_at,application_id LIMIT 1")) return fail(query.lastError().text());
    if(!query.next()) return true;return application(query.value(0).toInt(),result);
}
bool Database::notifications(QList<NotificationRecord> &result)
{
    lastError.clear();result.clear();AppSession::Identity user;if(!currentUser(user)) return false;
    QSqlQuery query(db());
    if(!execute(query,"SELECT notification_id,title,message,notification_type,created_at,is_read FROM notifications WHERE user_id=:user ORDER BY created_at DESC,notification_id DESC",{{"user",user.userId}})) return false;
    while(query.next()) result.append({query.value(0).toInt(),query.value(1).toString(),query.value(2).toString(),query.value(3).toString(),query.value(4).toDateTime().toTimeZone(QTimeZone("Asia/Colombo")),query.value(5).toBool()});return true;
}
bool Database::markNotificationRead(int id)
{
    lastError.clear();AppSession::Identity user;if(!currentUser(user)) return false;
    QSqlQuery query(db());
    if(!execute(query,"SELECT notification_id FROM notifications WHERE notification_id=:id AND user_id=:user",{{"id",id},{"user",user.userId}})) return false;
    if(!query.next()) return fail("The notification is not available for your account.");
    return execute(query,"UPDATE notifications SET is_read=1 WHERE notification_id=:id AND user_id=:user",{{"id",id},{"user",user.userId}});
}
bool Database::summary(ReportSummary &result)
{
    lastError.clear();result={};AppSession::Identity user;if(!currentUser(user)) return false;
    QString sql="SELECT COUNT(*),COALESCE(SUM(status='PENDING'),0),COALESCE(SUM(status='APPROVED'),0),COALESCE(SUM(status='DECLINED'),0) FROM medical_applications";
    QVariantMap values;if(user.role==AppSession::Role::Student) {sql+=" WHERE student_id=:student";values["student"]=user.studentId;}
    QSqlQuery query(db());if(!execute(query,sql,values) || !query.next()) return false;
    result={query.value(0).toInt(),query.value(1).toInt(),query.value(2).toInt(),query.value(3).toInt()};return true;
}
QString Database::applicationCode(int id) { return QString("APP-%1").arg(id,3,10,QChar('0')); }
QString Database::medicalCode(const QString &studentId,const QDate &submissionDate,qint64 dailyNumber)
{
    if(studentId.trimmed().isEmpty() || !submissionDate.isValid() || dailyNumber<=0) return {};
    return studentId+'-'+submissionDate.toString("MM-dd")+'-'+QString::number(dailyNumber).rightJustified(2,'0');
}
int Database::applicationId(const QString &code)
{
    static const QRegularExpression pattern("^APP-([0-9]+)$");const auto match=pattern.match(code);
    return match.hasMatch() ? match.captured(1).toInt() : 0;
}
