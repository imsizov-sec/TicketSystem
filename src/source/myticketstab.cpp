#include "src/header/myticketstab.h"
#include "ui/ui_myticketstab.h"
#include "src/header/createticketdialog.h"
#include "utils/header/utils.h"
#include "utils/header/mailservice.h"
#include "src/header/ticketcard.h"
#include "src/header/prioritydelegate.h"
#include "src/header/mainwindow.h"
#include "src/header/profiletab.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>
#include <QStackedLayout>
#include <QDateTime>

MyTicketsTab::MyTicketsTab(int userId_, const QString& role_, QTabWidget* tabWidget_, QWidget* parent)
    : QWidget(parent),
    ui(new Ui::MyTicketsTab),
    userId(userId_),
    userRole(role_),
    tabWidget(tabWidget_)
{
    ui->setupUi(this);
    // ui->doneGroup->setVisible(true);

    // Переключение отображения завершённых задач через QStackedLayout
    QStackedLayout* doneStack = qobject_cast<QStackedLayout*>(ui->doneStackedContainer->layout());
    Q_ASSERT(doneStack);  // Убедимся, что указатель валиден
    doneStack->setCurrentIndex(1);  // по умолчанию отображаем таблицу

    connect(ui->toggleDoneButton, &QToolButton::toggled, this, [=](bool checked) {
        doneStack->setCurrentIndex(checked ? 1 : 0);
        ui->toggleDoneButton->setText(checked ? "▼" : "▲");
    });

    // Основная таблица
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Название", "Проект", "Приоритет", "Статус"});
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->tableView->setFixedHeight(250);

    connect(ui->tableView, &QTableView::clicked, this, &MyTicketsTab::onTicketClicked);

    // Кнопка создания тикета
    if (userRole != "начальник") {
        ui->createTicketButton->hide();
    } else {
        connect(ui->createTicketButton, &QPushButton::clicked, this, &MyTicketsTab::onCreateTicketClicked);
    }

    // Завершённые задачи
    doneModel = new QStandardItemModel(this);
    doneModel->setHorizontalHeaderLabels({"Название", "Проект", "Приоритет", "Статус"});
    ui->doneTableView->setModel(doneModel);
    ui->doneTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->doneTableView->verticalHeader()->setVisible(false);
    ui->doneTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->doneTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->doneTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->doneTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->doneTableView->setFixedHeight(250);

    // ui->doneTableContainer->setVisible(true); // начальное состояние

    connect(ui->doneTableView, &QTableView::clicked, this, &MyTicketsTab::onTicketClicked);

    loadTickets();
}

MyTicketsTab::~MyTicketsTab() {
    delete ui;
}

void MyTicketsTab::onCreateTicketClicked() {
    CreateTicketDialog* dialog = new CreateTicketDialog(userId, this);
    connect(dialog, &CreateTicketDialog::ticketCreated, this, &MyTicketsTab::loadTickets);

    if (auto* mw = qobject_cast<MainWindow*>(window())) {
        if (mw->profileTab)
            connect(dialog, &CreateTicketDialog::ticketCreated, mw->profileTab, &ProfileTab::refreshStats);
        if (mw->projectsTab)
            connect(dialog, &CreateTicketDialog::ticketCreated, mw->projectsTab, &MyProjectsTab::loadProjects);
    }

    dialog->exec();
    delete dialog;
}

void MyTicketsTab::onTicketClicked(const QModelIndex& index) {
    if (!index.isValid()) return;

    int row = index.row();
    const QAbstractItemModel* source = index.model();
    int ticketId = -1;

    if (source == model)
        ticketId = model->item(row, 0)->data(Qt::UserRole).toInt();
    else if (doneModel && source == doneModel)
        ticketId = doneModel->item(row, 0)->data(Qt::UserRole).toInt();

    if (ticketId < 0) return;

    TicketCard* card = new TicketCard(ticketId, userId, tabWidget, this);
    tabWidget->addTab(card, QString("Тикет #%1").arg(ticketId));
    tabWidget->setCurrentWidget(card);

    connect(card, &TicketCard::ticketUpdated, this, &MyTicketsTab::loadTickets);

    if (auto* mw = qobject_cast<MainWindow*>(card->window()); mw && mw->profileTab)
        connect(card, &TicketCard::ticketUpdated, mw->profileTab, &ProfileTab::refreshStats);
}

void MyTicketsTab::loadTickets() {
    model->removeRows(0, model->rowCount());

    QSqlQuery query;
    query.prepare(loadSqlQuery(":/sql/sql/getUserTickets.sql"));
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        return;
    }

    QList<QList<QStandardItem*>> rows;
    QMap<QString, int> priorityOrder = {
        {"Немедленный", 0},
        {"Высокий", 1},
        {"Средний", 2},
        {"Низкий", 3}
    };

    while (query.next()) {
        int ticketId = query.value(0).toInt();
        QString priority = query.value(3).toString();

        QList<QStandardItem*> row;
        for (int i = 1; i <= 4; ++i) {
            QStandardItem* item = new QStandardItem(query.value(i).toString());
            if (i == 1)
                item->setData(ticketId, Qt::UserRole);
            row.append(item);
        }

        int order = priorityOrder.value(priority, 99);
        row.first()->setData(order, Qt::UserRole + 1);
        rows.append(row);
    }

    std::sort(rows.begin(), rows.end(), [](const QList<QStandardItem*>& a, const QList<QStandardItem*>& b) {
        return a.first()->data(Qt::UserRole + 1).toInt() < b.first()->data(Qt::UserRole + 1).toInt();
    });

    for (const QList<QStandardItem*>& row : rows)
        model->appendRow(row);

    ui->tableView->setItemDelegate(new PriorityDelegate(this));

    // Завершённые задачи
    doneModel->removeRows(0, doneModel->rowCount());

    QSqlQuery doneQuery;
    doneQuery.prepare(loadSqlQuery(":/sql/sql/getUserDoneTickets.sql"));
    doneQuery.bindValue(":userId", userId);

    if (doneQuery.exec()) {
        while (doneQuery.next()) {
            QList<QStandardItem*> row;
            int ticketId = doneQuery.value(0).toInt();
            for (int i = 1; i <= 4; ++i) {
                QStandardItem* item = new QStandardItem(doneQuery.value(i).toString());
                item->setBackground(QColor("#E8E8E8"));
                if (i == 1) {
                    QFont font = item->font();
                    font.setStrikeOut(true);
                    item->setFont(font);
                    item->setData(ticketId, Qt::UserRole);
                }
                row.append(item);
            }
            doneModel->appendRow(row);
        }
    }

    ui->doneTableView->setItemDelegate(new PriorityDelegate(this));
}

void MyTicketsTab::refreshTickets() {
    QSqlQuery roleQuery;
    roleQuery.prepare("SELECT role FROM users WHERE id = :userId");
    roleQuery.bindValue(":userId", userId);

    if (!roleQuery.exec() || !roleQuery.next()) {
        qWarning() << "Не удалось определить роль пользователя";
        loadTickets();
        return;
    }

    QString role = roleQuery.value(0).toString();
    if (role != "распределитель") {
        loadTickets();
        return;
    }

    MailService mailService("C:\\Users\\Hp\\Desktop\\TickteSystem\\cert.pem");
    QList<MailMessage> messages = mailService.fetchAllMail();

    QSqlQuery checkQuery, insertEmailQuery, insertTicketQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM processed_emails WHERE uid = :uid");
    insertEmailQuery.prepare("INSERT INTO processed_emails (uid) VALUES (:uid)");
    insertTicketQuery.prepare(R"(
        INSERT INTO tickets (title, description, status_id, assignee_id, creator_id, start_date, created_at)
        VALUES (:title, :description, 4, :assignee_id, :assignee_id, :start_date, :created_at)
    )");

    for (const MailMessage& msg : messages) {
        checkQuery.bindValue(":uid", msg.uid);
        if (!checkQuery.exec() || !checkQuery.next()) continue;
        if (checkQuery.value(0).toInt() > 0) continue;

        insertEmailQuery.bindValue(":uid", msg.uid);
        if (!insertEmailQuery.exec()) {
            qWarning() << "Ошибка вставки UID:" << insertEmailQuery.lastError().text();
            continue;
        }

        QString composedDescription = QString("Отправитель: %1\n\n%2").arg(msg.sender, msg.body);
        insertTicketQuery.bindValue(":title", msg.subject);
        insertTicketQuery.bindValue(":description", composedDescription); // Автор — распределитель
        insertTicketQuery.bindValue(":assignee_id", userId);  // Назначен тоже он
        insertTicketQuery.bindValue(":start_date", QDateTime::currentDateTime());
        insertTicketQuery.bindValue(":created_at", QDateTime::currentDateTime());
        if (!insertTicketQuery.exec()) {
            qWarning() << "Ошибка вставки тикета:" << insertTicketQuery.lastError().text();
            continue;
        }

        // TODO: Добавить привязку файлов, если необходимо
    }

    loadTickets();
}

