#ifndef USER_H
#define USER_H

#include <QString>

struct User {
    int id;
    QString fullName;
    QString email;
    QString role;
    QString department;

    User();
};

#endif // USER_H
