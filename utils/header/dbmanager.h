#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>

class DBManager {
public:
    static bool connect(const QString& host, const QString& dbName, const QString& user, const QString& password);
};

#endif // DBMANAGER_H
