#include "database.h"

#include <QSqlError>
#include <QDebug>

bool Database::connect()
{
    QSqlDatabase db;

    if (QSqlDatabase::contains("U_MAMS_CONNECTION"))
    {
        db = QSqlDatabase::database("U_MAMS_CONNECTION");
    }
    else
    {
        db = QSqlDatabase::addDatabase(
            "QMYSQL",
            "U_MAMS_CONNECTION"
            );
    }

    db.setHostName("localhost");
    db.setPort(3306);
    db.setDatabaseName("umams_db");
    db.setUserName("root");

    // IMPORTANT:
    // Replace this with your actual MySQL root password.
    db.setPassword("CHAma@123");

    if (!db.open())
    {
        qDebug() << "MySQL connection failed:";
        qDebug() << db.lastError().text();

        return false;
    }

    qDebug() << "MySQL connected successfully.";

    return true;
}

void Database::close()
{
    if (QSqlDatabase::contains("U_MAMS_CONNECTION"))
    {
        QSqlDatabase db =
            QSqlDatabase::database("U_MAMS_CONNECTION");

        if (db.isOpen())
        {
            db.close();
        }
    }
}