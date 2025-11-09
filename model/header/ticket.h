#ifndef TICKET_H
#define TICKET_H

#include <QString>
#include <QStringList>

class Ticket {
public:
    Ticket();

    bool load(int id);  // Загрузка тикета по ID из БД

    int id;
    QString title;
    QString description;
    QString project;
    QString tracker;
    QString status;
    QString priority;
    QString assignee;
    QString watcher;
    QStringList attachments;
    QString createdAt;
};

#endif // TICKET_H
