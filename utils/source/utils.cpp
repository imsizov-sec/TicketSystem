#include "utils/header/utils.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

QString loadSqlQuery(const QString &filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Файл не найден в ресурсах:" << filePath;
        return {};
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть SQL-файл:" << filePath;
        return {};
    }

    QTextStream in(&file);
    return in.readAll();
}

