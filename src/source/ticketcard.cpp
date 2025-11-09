#include "src/header/ticketcard.h"
#include "model/header/ticket.h"
#include "utils/header/utils.h"
#include "utils/header/storagemanager.h"
#include <QListWidgetItem>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QScrollBar>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QtPrintSupport/QPrinter>

TicketCard::TicketCard(int ticketId, int userId, QTabWidget* tabWidget, QWidget* parent)
    : QWidget(parent), ticketId(ticketId), userId(userId), tabWidget(tabWidget) {
    ui.setupUi(this);

    if (!ticket.load(ticketId)) {
        close();
        return;
    }

    reloadTicketDetailsFromDatabase();

    // Устанавливаем заголовок тикета
    QString fullTitle = QString("№%1  <b>%2</b>  (<i>%3</i>)")
                            .arg(ticket.id)
                            .arg(ticket.title)
                            .arg(ticket.tracker);
    ui.ticketTitleLabel->setText(fullTitle);
    ui.ticketTitleLabel->setAlignment(Qt::AlignCenter);

    // Описание тикета
    ui.ticketDescription->setPlainText(ticket.description);
    ui.ticketDescription->setReadOnly(true);
    ui.ticketDescription->document()->setTextWidth(ui.ticketDescription->viewport()->width());

    int maxHeight = 150;
    int contentHeight = static_cast<int>(ui.ticketDescription->document()->size().height());
    int finalHeight = qMin(contentHeight + 10, maxHeight);
    ui.ticketDescription->setMinimumHeight(finalHeight);
    ui.ticketDescription->setMaximumHeight(finalHeight);

    // Информация
    ui.labelProjectValue->setText(ticket.project);
    ui.labelTrackerValue->setText(ticket.tracker);
    ui.labelStatusValue->setText(ticket.status);
    ui.labelPriorityValue->setText(ticket.priority);
    ui.labelAssignedValue->setText(ticket.assignee);
    ui.labelObserverValue->setText(ticket.watcher);

    // Видимость редактирования
    ui.editPanel->setVisible(false);
    ui.editFooterPanel->setVisible(false);
    ui.attachFileButton->hide();
    ui.attachedFilesList->hide();

    // Подключения
    connect(ui.editButton, &QPushButton::clicked, this, &TicketCard::onEditClicked);
    connect(ui.backButton, &QPushButton::clicked, this, &TicketCard::onBackClicked);
    connect(ui.saveButton, &QPushButton::clicked, this, &TicketCard::onSaveClicked);
    connect(ui.attachFileButton, &QPushButton::clicked, this, &TicketCard::onAttachFileClicked);
    connect(ui.closeButton, &QPushButton::clicked, this, &TicketCard::onBackClicked);
    connect(ui.pdfButton, &QPushButton::clicked, this, &TicketCard::onSavePdfClicked);
    connect(ui.completeButton, &QPushButton::clicked, this, &TicketCard::onCompleteClicked);

    loadHistory();
    loadFiles();
}

void TicketCard::onEditClicked() {
    bool isVisible = ui.editPanel->isVisible();
    ui.editPanel->setVisible(!isVisible);
    ui.editFooterPanel->setVisible(!isVisible);
    ui.attachFileButton->setVisible(!isVisible);
    ui.attachedFilesList->setVisible(!isVisible);

    ui.attachedFilesList->clear();
    for (const QString& file : newlyAttachedFiles)
        ui.attachedFilesList->addItem(file);

    if (!isVisible) {
        ui.editTitle->setText(ticket.title);

        QString sql;

        sql = loadSqlQuery(":/sql/sql/getStatusesIdAndName.sql");
        QSqlQuery queryStatus(sql);
        ui.editStatus->clear();
        statusMap.clear();
        while (queryStatus.next()) {
            int id = queryStatus.value("id").toInt();
            QString name = queryStatus.value("name").toString();
            ui.editStatus->addItem(name);
            statusMap[name] = id;
        }
        ui.editStatus->setCurrentText(ticket.status);

        sql = loadSqlQuery(":/sql/sql/getPrioritiesIdAndName.sql");
        QSqlQuery queryPriority(sql);
        ui.editPriority->clear();
        priorityMap.clear();
        while (queryPriority.next()) {
            int id = queryPriority.value("id").toInt();
            QString name = queryPriority.value("name").toString();
            ui.editPriority->addItem(name);
            priorityMap[name] = id;
        }
        ui.editPriority->setCurrentText(ticket.priority);

        QString currentUserRole;
        QString roleSql = loadSqlQuery(":/sql/sql/getUserRoleById.sql");
        QSqlQuery roleQuery;
        roleQuery.prepare(roleSql);
        roleQuery.bindValue(":userId", userId);
        if (roleQuery.exec() && roleQuery.next()) {
            currentUserRole = roleQuery.value("role").toString();
        }

        if (currentUserRole == "распределитель") {
            sql = loadSqlQuery(":/sql/sql/getBossesIdAndFullName.sql");
        } else {
            sql = loadSqlQuery(":/sql/sql/getUsersIdAndFullName.sql");
        }

        QSqlQuery queryUsers(sql);
        ui.editAssignee->clear();
        ui.editWatcher->clear();
        userMap.clear();

        while (queryUsers.next()) {
            int id = queryUsers.value("id").toInt();
            QString name = queryUsers.value("full_name").toString();
            ui.editAssignee->addItem(name);
            ui.editWatcher->addItem(name);
            userMap[name] = id;
        }

        ui.editAssignee->setCurrentText(ticket.assignee);
        ui.editWatcher->setCurrentText(ticket.watcher);

        sql = loadSqlQuery(":/sql/sql/getTrackersIdAndName.sql");
        QSqlQuery queryTrackers(sql);
        ui.editTracker->clear();
        trackerMap.clear();
        while (queryTrackers.next()) {
            int id = queryTrackers.value("id").toInt();
            QString name = queryTrackers.value("name").toString();
            ui.editTracker->addItem(name);
            trackerMap[name] = id;
        }
        ui.editTracker->setCurrentText(ticket.tracker);

        QTimer::singleShot(0, this, [this]() {
            if (QScrollBar* scrollBar = ui.mainScrollArea->verticalScrollBar()) {
                scrollBar->setValue(scrollBar->maximum());
            }
        });
    }
}

void TicketCard::onSaveClicked() {
    QString newTitle = ui.editTitle->text();
    QString newStatus = ui.editStatus->currentText();
    QString newPriority = ui.editPriority->currentText();
    QString newAssignee = ui.editAssignee->currentText();
    QString newWatcher = ui.editWatcher->currentText();
    QString newTracker = ui.editTracker->currentText();
    QString comment = ui.commentEdit->text().trimmed();

    int statusId = statusMap.value(newStatus, -1);
    int priorityId = priorityMap.value(newPriority, -1);
    int assigneeId = userMap.value(newAssignee, -1);
    int watcherId = userMap.value(newWatcher, -1);
    int trackerId = trackerMap.value(newTracker, -1);

    QString sql = loadSqlQuery(":/sql/sql/updateTicketInfo.sql");
    QSqlQuery updateQuery;
    updateQuery.prepare(sql);
    updateQuery.bindValue(":title", newTitle);
    updateQuery.bindValue(":statusId", statusId);
    updateQuery.bindValue(":priorityId", priorityId);
    updateQuery.bindValue(":assigneeId", assigneeId);
    updateQuery.bindValue(":watcherId", watcherId);
    updateQuery.bindValue(":trackerId", trackerId);
    updateQuery.bindValue(":ticketId", ticketId);

    if (!updateQuery.exec()) {
        return;
    }

    QStringList changes;
    QStringList fileLinks;
    auto track = [&](const QString& field, const QString& oldVal, const QString& newVal) {
        if (oldVal != newVal) {
            changes << QString("Параметр <b>%1</b> изменился с <i>%2</i> на <b>%3</b>").arg(field, oldVal, newVal);
        }
    };

    track("Название", ticket.title, newTitle);
    track("Статус", ticket.status, newStatus);
    track("Приоритет", ticket.priority, newPriority);
    track("Назначена", ticket.assignee, newAssignee);
    track("Наблюдатель", ticket.watcher, newWatcher);
    track("Трекер", ticket.tracker, newTracker);

    QString sqlFileInsert = loadSqlQuery(":/sql/sql/saveTicketFile.sql");
    QSqlQuery insertQuery;

    for (const QString& file : newlyAttachedFiles) {
        insertQuery.prepare(sqlFileInsert);
        insertQuery.bindValue(":ticketId", ticketId);
        insertQuery.bindValue(":fileName", file);
        insertQuery.bindValue(":relativePath", "ticketFiles/" + file);
        if (!insertQuery.exec()) {
            qDebug() << "Ошибка сохранения файла в БД:" << file << insertQuery.lastError().text();
        } else {
            fileLinks << QString("Файл <a href='%1'>%2</a> добавлен").arg(file, file);
        }
    }
    newlyAttachedFiles.clear();

    if (!changes.isEmpty() || !comment.isEmpty() || !fileLinks.isEmpty()) {
        QStringList fullChanges = changes;
        fullChanges.append(fileLinks);

        QString historySql = loadSqlQuery(":/sql/sql/saveTicketHistory.sql");
        QSqlQuery historyQuery;
        historyQuery.prepare(historySql);
        historyQuery.bindValue(":ticketId", ticketId);
        historyQuery.bindValue(":userId", userId);
        historyQuery.bindValue(":summary", fullChanges.join("<br>"));
        historyQuery.bindValue(":comment", comment);
        historyQuery.bindValue(":changedAt", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        historyQuery.exec();
    }

    ticket.title = newTitle;
    ticket.status = newStatus;
    ticket.priority = newPriority;
    ticket.assignee = newAssignee;
    ticket.watcher = newWatcher;
    ticket.tracker = newTracker;

    ui.labelStatusValue->setText(newStatus);
    ui.labelPriorityValue->setText(newPriority);
    ui.labelAssignedValue->setText(newAssignee);
    ui.labelObserverValue->setText(newWatcher);
    ui.labelTrackerValue->setText(newTracker);
    ui.ticketTitleLabel->setText(QString("№%1  <b>%2</b>  (<i>%3</i>)").arg(ticket.id).arg(newTitle).arg(newTracker));

    ui.editPanel->setVisible(false);
    ui.editFooterPanel->setVisible(false);
    ui.commentEdit->clear();

    if (!newlyAttachedFiles.isEmpty()) {
        QString sql = loadSqlQuery(":/sql/sql/saveTicketFile.sql");
        QSqlQuery insertQuery;
        for (const QString& file : newlyAttachedFiles) {
            insertQuery.prepare(sql);
            insertQuery.bindValue(":ticketId", ticketId);
            insertQuery.bindValue(":fileName", file);
            insertQuery.bindValue(":relativePath", "ticketFiles/" + file);
            if (!insertQuery.exec()) {
                qDebug() << "Ошибка сохранения файла в БД:" << file << insertQuery.lastError().text();
            } else {
                fileLinks << QString("Файл <a href='%1'>%2</a> добавлен").arg(file, file);
            }
        }
    }

    emit ticketUpdated();
    loadHistory();
    loadFiles();
}

void TicketCard::onAttachFileClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, tr("Выберите файл для прикрепления"));
    if (filePath.isEmpty()) return;

    QFileInfo fi(filePath);
    QString storedPath = StorageManager::saveToStorage(filePath, "ticketFiles", fi.fileName());

    if (!storedPath.isEmpty()) {
        // Только в список отложенных
        newlyAttachedFiles.append(fi.fileName());

        if (ui.editPanel->isVisible())
            ui.attachedFilesList->addItem(fi.fileName());
    } else {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось скопировать файл в storage."));
    }
}

void TicketCard::onBackClicked() {
    emit ticketUpdated();

    if (!tabWidget) {
        return;
    }

    int myIndex = tabWidget->indexOf(this);

    // Переключаемся на "Мои тикеты"
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (tabWidget->tabText(i) == "Мои тикеты") {
            tabWidget->setCurrentIndex(i);
            break;
        }
    }

    if (myIndex != -1) {
        tabWidget->removeTab(myIndex);
    }
}


void TicketCard::loadHistory() {
    QString sql = loadSqlQuery(":/sql/sql/getTicketHistoryByTicketId.sql");
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":ticketId", ticketId);
    if (!query.exec()) return;

    QLayoutItem* child;
    while ((child = ui.historyLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    QList<QWidget*> blocks;

    while (query.next()) {
        QString user = query.value("user").toString();
        QDateTime dt = query.value("changed_at").toDateTime();
        QString formattedTime = dt.toString("dd.MM.yyyy HH:mm");
        QString summary = query.value("changes_summary").toString();
        QString comment = query.value("comment").toString();

        QWidget* historyItemWidget = new QWidget;
        historyItemWidget->setStyleSheet("background: white; border: 1px solid #ccc; border-radius: 5px; margin-bottom: 20px;");
        QVBoxLayout* historyItemLayout = new QVBoxLayout(historyItemWidget);
        historyItemLayout->setContentsMargins(8, 8, 8, 8);

        QLabel* topLabel = new QLabel(
            QString("<b>%1</b> <span style='color:gray'>(%2)</span><br>%3")
                .arg(user, formattedTime, summary)
            );
        topLabel->setWordWrap(true);
        topLabel->setTextFormat(Qt::RichText);
        topLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);  // Важно
        topLabel->setOpenExternalLinks(false);                          // Обрабатываем вручную
        topLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        topLabel->setStyleSheet("background: #f5f5f5; border: 1px solid #ccc; border-radius: 5px; padding: 6px;");
        historyItemLayout->addWidget(topLabel);

        connect(topLabel, &QLabel::linkActivated, this, [this](const QString& link) {
            QString sql = loadSqlQuery(":/sql/sql/getTicketFilePath.sql");
            QSqlQuery q;
            q.prepare(sql);
            q.bindValue(":ticketId", ticketId);
            q.bindValue(":fileName", link);
            if (q.exec() && q.next()) {
                StorageManager::downloadTo(q.value("relative_path").toString(), this);
            }
        });

        if (!comment.isEmpty()) {
            QTextEdit* commentText = new QTextEdit(comment);
            commentText->setReadOnly(true);
            commentText->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            commentText->setStyleSheet("background: #f5f5f5; border: 1px solid #ccc; border-radius: 5px; padding: 6px;");
            commentText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
            commentText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            commentText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            commentText->document()->setTextWidth(commentText->viewport()->width());
            int calculatedHeight = int(commentText->document()->size().height()) + 12;
            commentText->setMinimumHeight(std::max(calculatedHeight, 60));
            commentText->setMaximumHeight(std::max(calculatedHeight, 60));
            historyItemLayout->addWidget(commentText);
        }

        blocks.prepend(historyItemWidget);
    }

    for (QWidget* w : blocks)
        ui.historyLayout->addWidget(w);
}


void TicketCard::loadFiles() {
    // Очистка layout
    QLayoutItem* item;
    while ((item = ui.filesLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            delete item->widget();
        delete item;
    }

    // Добавление spacer-а в конец для "прижатия вверх"
    ui.filesLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Загрузка файлов и добавление в layout перед spacer-ом
    QString sql = loadSqlQuery(":/sql/sql/getTicketFilesByTicketId.sql");
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":ticketId", ticketId);
    if (query.exec()) {
        while (query.next()) {
            const QString fileName = query.value("file_name").toString();

            QLabel* label = new QLabel(QString("<b><font color='blue'>%1</font></b>").arg(fileName));
            label->setCursor(Qt::PointingHandCursor);
            label->setStyleSheet("QLabel:hover { text-decoration: underline; }");
            label->installEventFilter(this);
            label->setProperty("fileName", fileName);

            // Вставить перед spacer-ом (последний элемент)
            ui.filesLayout->insertWidget(ui.filesLayout->count() - 1, label);
        }
    }

    int totalHeight = 0;
    for (int i = 0; i < ui.filesLayout->count(); ++i) {
        QLayoutItem* item = ui.filesLayout->itemAt(i);
        if (QWidget* w = item->widget())
            totalHeight += w->sizeHint().height();
    }

    ui.filesBox->setMinimumHeight(std::max(40, totalHeight + 20));
}

void TicketCard::addFileLabel(const QString& fileName) {
    QLabel* label = new QLabel(QString("<b><font color='blue'>%1</font></b>").arg(fileName));
    label->setCursor(Qt::PointingHandCursor);
    label->setStyleSheet("QLabel:hover { text-decoration: underline; }");
    label->installEventFilter(this);
    label->setProperty("fileName", fileName);
    ui.filesLayout->insertWidget(0, label);
}

bool TicketCard::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QLabel* label = qobject_cast<QLabel*>(obj);
        if (label) {
            QString sql = loadSqlQuery(":/sql/sql/getTicketFilePath.sql");
            QSqlQuery q;
            q.prepare(sql);
            q.bindValue(":ticketId", ticketId);
            q.bindValue(":fileName", label->property("fileName").toString());
            if (q.exec() && q.next())
                StorageManager::downloadTo(q.value("relative_path").toString(), this);
        }
    }
    return QWidget::eventFilter(obj, event);
}

QString TicketCard::stripHtmlTags(const QString& html) {
    QString text = html;
    text.replace(QRegularExpression("<[^>]*>"), "");
    text.replace("&nbsp;", " ");
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&amp;", "&");
    text.replace("<br>", "\n", Qt::CaseInsensitive);
    return text;
}

void TicketCard::onSavePdfClicked() {
    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить как PDF", "ticket.pdf", "PDF files (*.pdf)");
    if (filePath.isEmpty())
        return;

    QFile file(":/templates/templates/ticket_template.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить HTML шаблон");
        return;
    }

    QString htmlTemplate = QString::fromUtf8(file.readAll());
    file.close();

    QString historyHtml;
    QSqlQuery query;
    QString sql = loadSqlQuery(":/sql/sql/getTicketHistoryByTicketId.sql");
    query.prepare(sql);
    query.bindValue(":ticketId", ticketId);
    if (query.exec()) {
        while (query.next()) {
            QString user = query.value("user").toString();
            QDateTime dt = query.value("changed_at").toDateTime();
            QString summaryRaw = stripHtmlTags(query.value("changes_summary").toString());
            QStringList lines;

            // Разбиваем изменения по параметрам
            const QString marker = "Параметр ";
            for (const QString& part : summaryRaw.split(marker, Qt::SkipEmptyParts)) {
                lines << "<div>Параметр " + part.trimmed() + "</div>";
            }

            QString formattedSummary = lines.join("\n");

            QString comment = stripHtmlTags(query.value("comment").toString());
            QString commentBlock;
            if (!comment.isEmpty()) {
                commentBlock = QString("<div class='history-comment'>%1</div>").arg(comment.toHtmlEscaped());
            }

            historyHtml.prepend(QString(R"(
                <div class="history-block">
                    <div class="history-header">%1 <span style="color:gray;">(%2)</span></div>
                    <div>%3</div>
                    %4
                </div>
            )")
            .arg(user.toHtmlEscaped(),
                 dt.toString("dd.MM.yyyy HH:mm"),
                 formattedSummary,
                 commentBlock));
        }
    }

    QMap<QString, QString> values = {
        {"{{id}}", QString::number(ticket.id)},
        {"{{title}}", ticket.title.toHtmlEscaped()},
        {"{{tracker}}", ticket.tracker.toHtmlEscaped()},
        {"{{description}}", ticket.description.toHtmlEscaped()},
        {"{{project}}", ticket.project.toHtmlEscaped()},
        {"{{status}}", ticket.status.toHtmlEscaped()},
        {"{{priority}}", ticket.priority.toHtmlEscaped()},
        {"{{assignee}}", ticket.assignee.toHtmlEscaped()},
        {"{{watcher}}", ticket.watcher.toHtmlEscaped()},
        {"{{history}}", historyHtml}
    };

    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        htmlTemplate.replace(it.key(), it.value());

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageMargins(QMarginsF(10, 10, 10, 10));

    QTextDocument doc;
    doc.setHtml(htmlTemplate);
    doc.print(&printer);
}

void TicketCard::reloadTicketDetailsFromDatabase() {
    QString sql = loadSqlQuery(":/sql/sql/getFullTicketInfoById.sql");
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":ticketId", ticketId);
    if (!query.exec() || !query.next()) {
        qWarning() << "Ошибка загрузки тикета по id:" << query.lastError().text();
        return;
    }

    ticket.project = query.value("project").toString();
    ticket.tracker = query.value("tracker").toString();
    ticket.status = query.value("status").toString();
    ticket.priority = query.value("priority").toString();
    ticket.assignee = query.value("assignee").toString();   // ФИО
    ticket.watcher = query.value("watcher").toString();     // ФИО
}

void TicketCard::onCompleteClicked() {
    QString newStatus = "Завершён";
    if (ticket.status == newStatus)
        return; // Уже завершён

    int statusId = -1;
    QString sql = loadSqlQuery(":/sql/sql/getStatusesIdAndName.sql");
    QSqlQuery statusQuery(sql);
    while (statusQuery.next()) {
        if (statusQuery.value("name").toString() == newStatus) {
            statusId = statusQuery.value("id").toInt();
            break;
        }
    }

    if (statusId == -1) {
        QMessageBox::warning(this, "Ошибка", "Статус 'Завершён' не найден в базе.");
        return;
    }

    sql = loadSqlQuery(":/sql/sql/updateTicketStatusOnly.sql");  // создайте такой SQL: update tickets set status_id = :statusId where id = :ticketId
    QSqlQuery updateQuery;
    updateQuery.prepare(sql);
    updateQuery.bindValue(":statusId", statusId);
    updateQuery.bindValue(":ticketId", ticketId);
    if (!updateQuery.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить статус тикета.");
        return;
    }

    // История
    QString change = QString("Параметр <b>Статус</b> изменился с <i>%1</i> на <b>%2</b>").arg(ticket.status, newStatus);
    sql = loadSqlQuery(":/sql/sql/saveTicketHistory.sql");
    QSqlQuery histQuery;
    histQuery.prepare(sql);
    histQuery.bindValue(":ticketId", ticketId);
    histQuery.bindValue(":userId", userId);
    histQuery.bindValue(":summary", change);
    histQuery.bindValue(":comment", "");
    histQuery.bindValue(":changedAt", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    histQuery.exec();

    ticket.status = newStatus;
    ui.labelStatusValue->setText(newStatus);
    loadHistory();

    emit ticketUpdated();
}
