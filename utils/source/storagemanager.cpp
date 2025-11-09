#include "utils/header/storagemanager.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QCoreApplication>

QString StorageManager::basePath() {
    static QString path = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../storage");
    QDir().mkpath(path);  // убедимся, что директория существует
    return QDir(path).absolutePath();
}

QString StorageManager::photosPath() {
    QString path = basePath() + "/photos";
    QDir().mkpath(path);
    return path;
}

QString StorageManager::ticketsPath() {
    QString path = basePath() + "/ticketFiles";
    QDir().mkpath(path);
    return path;
}

QString StorageManager::saveToStorage(const QString& sourcePath, const QString& category, const QString& preferredName) {
    QString targetDir = basePath() + "/" + category;
    QDir().mkpath(targetDir);

    QFileInfo srcInfo(sourcePath);
    QString targetName = preferredName.isEmpty() ? srcInfo.fileName() : preferredName;
    QString destPath = targetDir + "/" + targetName;

    if (QFile::exists(destPath))
        QFile::remove(destPath);

    if (QFile::copy(sourcePath, destPath))
        return category + "/" + targetName;
    else
        return QString();
}

QString StorageManager::getAbsolutePath(const QString& relativePath) {
    return basePath() + "/" + relativePath;
}

bool StorageManager::downloadTo(const QString& relativePath, QWidget* parent) {
    QString absPath = getAbsolutePath(relativePath);
    QFileInfo fileInfo(absPath);
    if (!fileInfo.exists())
        return false;

    QString savePath = QFileDialog::getSaveFileName(parent, "Сохранить как", fileInfo.fileName());
    if (savePath.isEmpty())
        return false;

    if (!QFile::copy(absPath, savePath)) {
        QMessageBox::warning(parent, "Ошибка", "Не удалось сохранить файл.");
        return false;
    }

    return true;
}
