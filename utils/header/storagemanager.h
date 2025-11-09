#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>
#include <QWidget>

class StorageManager {
public:
    static QString basePath();
    static QString photosPath();
    static QString ticketsPath();

    static QString saveToStorage(const QString& sourcePath, const QString& category, const QString& preferredName = "");
    static QString getAbsolutePath(const QString& relativePath);
    static bool downloadTo(const QString& relativePath, QWidget* parent);
};

#endif // STORAGEMANAGER_H
