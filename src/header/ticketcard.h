#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QMap>
#include <QStringList>
#include "ui/ui_ticketcard.h"
#include "model/header/ticket.h"

class TicketCard : public QWidget {
    Q_OBJECT

public:
    explicit TicketCard(int ticketId, int userId, QTabWidget* tabWidget, QWidget* parent = nullptr);

signals:
    void ticketUpdated();
    void ticketCreated();

private:
    Ui::TicketCard ui;
    Ticket ticket;
    int ticketId;
    int userId;
    QTabWidget* tabWidget;
    QStringList newlyAttachedFiles;

    QMap<QString, int> statusMap;
    QMap<QString, int> priorityMap;
    QMap<QString, int> userMap;
    QMap<QString, int> trackerMap;

    void addFileLabel(const QString& fileName);
    void loadHistory();
    void loadFiles();
    bool eventFilter(QObject* obj, QEvent* event) override;
    QString stripHtmlTags(const QString& html);

    void reloadTicketDetailsFromDatabase();

private slots:
    void onEditClicked();
    void onBackClicked();
    void onSaveClicked();
    void onAttachFileClicked();
    void onSavePdfClicked();
    void onCompleteClicked();
};
