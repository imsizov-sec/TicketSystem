#include "utils/header/dbmanager.h"
#include <QtSql/QSqlDatabase>
#include <QDebug>
#include <QtSql/QSqlError>

bool DBManager::connect(const QString& host, const QString& dbName, const QString& user, const QString& password) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMARIADB");
    db.setHostName(host);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);
    if (!db.open()) {
        return false;
    }
    return true;
}
