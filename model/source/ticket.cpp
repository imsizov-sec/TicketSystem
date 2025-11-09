#include "model/header/ticket.h"
#include "utils/header/utils.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

Ticket::Ticket() {}

bool Ticket::load(int id_) {
    id = id_;
    QSqlQuery query;
    QString sql = loadSqlQuery(":/sql/sql/getFullTicketInfoById.sql");
    query.prepare(sql);
    query.bindValue(":ticketId", id);

    if (!query.exec()) {
        return false;
    }

    if (query.next()) {
        title = query.value("title").toString();
        description = query.value("description").toString();
        project = query.value("project").toString();
        tracker = query.value("tracker").toString();
        status = query.value("status").toString();
        priority = query.value("priority").toString();
        assignee = query.value("assignee").toString();
        watcher = query.value("watcher").toString();
        attachments = query.value("attachment").toString().split(';', Qt::SkipEmptyParts);
        createdAt = query.value("created_at").toString();
        return true;
    }

    return false;
}
