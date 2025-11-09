#pragma once

#include <QString>
#include <QList>

struct MailMessage {
    QString subject;
    QString sender;
    QString body;
    QString uid;
    // при необходимости: QList<QByteArray> attachments;
};

class MailService {
public:
    MailService(const QString& certPath);

    QList<MailMessage> fetchAllMail();

private:
    QString certPath;
};
