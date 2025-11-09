#ifndef MYTICKETSTAB_H
#define MYTICKETSTAB_H

#include <QWidget>
#include <QTabWidget>
#include <QStandardItemModel>
#include "ui/ui_myticketstab.h"

namespace Ui {
class MyTicketsTab;
}

class MyTicketsTab : public QWidget {
    Q_OBJECT

public:
    explicit MyTicketsTab(int userId, const QString& role, QTabWidget* tabWidget, QWidget* parent = nullptr);
    ~MyTicketsTab();
    void loadTickets();

private slots:
    void onCreateTicketClicked();
    void onTicketClicked(const QModelIndex& index);

private:
    Ui::MyTicketsTab* ui;

    int userId;
    QString userRole;
    QTabWidget* tabWidget;

    QStandardItemModel* model = nullptr;
    QStandardItemModel* doneModel = nullptr;
public slots:
    void refreshTickets();
signals:
    void ticketsChanged();
};

#endif // MYTICKETSTAB_H
