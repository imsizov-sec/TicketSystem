
#include "src/header/createticketdialog.h"
#include "ui/ui_createticketdialog.h"
#include "utils/header/utils.h"
#include "utils/header/storagemanager.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QDesktopServices>

CreateTicketDialog::CreateTicketDialog(int currentUserId, QWidget *parent)
    : QDialog(parent), ui(new Ui::CreateTicketDialog), userId(currentUserId) {
    ui->setupUi(this);
    db = QSqlDatabase::database();
    ui->startDateEdit->setDate(QDate::currentDate());
    populateCombos();

    connect(ui->projectCombo, &QComboBox::currentTextChanged, this, &CreateTicketDialog::updateWatchersByProject);
}

CreateTicketDialog::~CreateTicketDialog() {
    delete ui;
}

void CreateTicketDialog::populateCombos() {
    QSqlQuery query;
    QString sql;

    sql = loadSqlQuery(":/sql/sql/getProjectsIdAndName.sql");
    query.exec(sql);
    while (query.next())
        ui->projectCombo->addItem(query.value("name").toString(), query.value("id"));

    sql = loadSqlQuery(":/sql/sql/getTrackersIdAndName.sql");
    query.exec(sql);
    while (query.next())
        ui->trackerCombo->addItem(query.value("name").toString(), query.value("id"));

    sql = loadSqlQuery(":/sql/sql/getStatusesWithoutEnding.sql");
    query.exec(sql);
    while (query.next())
        ui->statusCombo->addItem(query.value("name").toString(), query.value("id"));

    sql = loadSqlQuery(":/sql/sql/getPrioritiesIdAndName.sql");
    query.exec(sql);
    while (query.next())
        ui->priorityCombo->addItem(query.value("name").toString(), query.value("id"));

    updateWatchersByProject();
}

void CreateTicketDialog::updateWatchersByProject() {
    ui->watcherCombo->clear();
    ui->assigneeCombo->clear();

    int projectId = ui->projectCombo->currentData().toInt();
    QSqlQuery query;

    QString sql = loadSqlQuery(":/sql/sql/getDepartmentIdByProject.sql");
    query.prepare(sql);
    query.bindValue(":projectId", projectId);
    if (!query.exec() || !query.next()) {
        showError("Ошибка получения отдела проекта.");
        return;
    }

    int deptId = query.value("department_id").toInt();
    sql = loadSqlQuery(":/sql/sql/getUsersByDepartment.sql");
    query.prepare(sql);
    query.bindValue(":departmentId", deptId);
    if (query.exec()) {
        while (query.next()) {
            QString name = query.value("full_name").toString();
            int id = query.value("id").toInt();
            ui->watcherCombo->addItem(name, id);
            ui->assigneeCombo->addItem(name, id);
        }
    }
}

void CreateTicketDialog::on_confirmButton_clicked() {
    QString title = ui->titleEdit->text().trimmed();
    QString tags = ui->tagEdit->text().trimmed();
    QString description = ui->descriptionEdit->toPlainText();

    if (title.isEmpty()) {
        showError("Введите название тикета.");
        return;
    }

    // Форматирование тегов
    QStringList tagList = tags.split(',', Qt::SkipEmptyParts);
    for (QString& tag : tagList)
        tag = "#" + tag.trimmed();
    QString fullTitle = title;
    if (!tagList.isEmpty())
        fullTitle += " " + tagList.join(" ");

    int projectId = ui->projectCombo->currentData().toInt();
    int trackerId = ui->trackerCombo->currentData().toInt();
    int statusId = ui->statusCombo->currentData().toInt();
    int priorityId = ui->priorityCombo->currentData().toInt();
    int assigneeId = ui->assigneeCombo->currentData().toInt();
    int watcherId = ui->watcherCombo->currentData().toInt();
    QDate startDate = ui->startDateEdit->date();

    QString sql = loadSqlQuery(":/sql/sql/saveTicket.sql");
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":title", fullTitle);
    query.bindValue(":description", description);
    query.bindValue(":projectId", projectId);
    query.bindValue(":trackerId", trackerId);
    query.bindValue(":statusId", statusId);
    query.bindValue(":priorityId", priorityId);
    query.bindValue(":assigneeId", assigneeId);
    query.bindValue(":watcherId", watcherId);
    query.bindValue(":creatorId", userId);
    query.bindValue(":startDate", startDate);

    if (!query.exec()) {
        showError("Ошибка при создании тикета: " + query.lastError().text());
        return;
    }

    int ticketId = query.lastInsertId().toInt();

    if (!newlyAttachedFiles.isEmpty()) {
        QString fileSql = loadSqlQuery(":/sql/sql/saveTicketFile.sql");
        QSqlQuery fileQuery;
        for (const QString &file : newlyAttachedFiles) {
            fileQuery.prepare(fileSql);
            fileQuery.bindValue(":ticketId", ticketId);
            fileQuery.bindValue(":fileName", file);
            fileQuery.bindValue(":relativePath", "ticketFiles/" + file);
        }
    }

    emit ticketCreated();
    emit ticketSuccessfullyCreated();
    accept();
}

void CreateTicketDialog::on_cancelBtn_clicked() {
    reject();
}

void CreateTicketDialog::on_attachBtn_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл");
    if (filePath.isEmpty()) return;

    QFileInfo fi(filePath);
    QString storedPath = StorageManager::saveToStorage(filePath, "ticketFiles", fi.fileName());

    if (!storedPath.isEmpty()) {
        newlyAttachedFiles.append(fi.fileName());
        addFileLabel(fi.fileName());
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось скопировать файл в storage.");
    }
}

void CreateTicketDialog::addFileLabel(const QString &fileName) {
    QLabel *label = new QLabel(QString("<b><font color='blue'>%1</font></b>").arg(fileName));
    label->setCursor(Qt::PointingHandCursor);
    label->setStyleSheet("QLabel:hover { text-decoration: underline; }");
    label->setProperty("fileName", fileName);
    label->installEventFilter(this);  // Важно

    ui->attachedFilesLayout->addWidget(label);
}

bool CreateTicketDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QLabel *label = qobject_cast<QLabel *>(obj);
        if (label) {
            const QString fileName = label->property("fileName").toString();

            // Путь должен быть точный, как в TicketCard
            const QString relativePath = "ticketFiles/" + fileName;
            StorageManager::downloadTo(relativePath, this);

            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

void CreateTicketDialog::showError(const QString &msg) {
    QMessageBox::critical(this, "Ошибка", msg);
}
