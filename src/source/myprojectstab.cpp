#include "src/header/myprojectstab.h"
#include "ui/ui_myprojectstab.h"
#include "utils/header/utils.h"
#include "src/header/createprojectdialog.h"
#include "src/header/ticketcard.h"
#include "src/header/editemployeedialog.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QInputDialog>
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

MyProjectsTab::MyProjectsTab(int userId_, const QString& role, QTabWidget* tabWidget_, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::MyProjectsTab),
    userId(userId_),
    userRole(role),
    tabWidget(tabWidget_)
{
    ui->setupUi(this);

    if (userRole != "начальник") {
        ui->createProjectButton->hide();
        ui->deleteProjectButton->hide();
    } else {
        connect(ui->createProjectButton, &QPushButton::clicked, this, &MyProjectsTab::onCreateProjectClicked);
        connect(ui->deleteProjectButton, &QPushButton::clicked, this, &MyProjectsTab::onDeleteProjectClicked);
        loadEmployees();  // Загрузка сотрудников отдела
    }

    loadProjects();
}

MyProjectsTab::~MyProjectsTab() {
    delete ui;
}

void MyProjectsTab::loadProjects() {
    QLayoutItem* item;
    while ((item = ui->projectsLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) delete w;
        delete item;
    }

    QSqlQuery query;
    QString sql = (userRole == "начальник")
                      ? loadSqlQuery(":/sql/sql/getChiefProjects.sql")
                      : loadSqlQuery(":/sql/sql/getEmployeeProjects.sql");

    query.prepare(sql);
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        return;
    }

    while (query.next()) {
        QString projectName = query.value("name").toString();
        int projectId = query.value("id").toInt();

        QWidget* projectWidget = new QWidget;
        QVBoxLayout* projectLayout = new QVBoxLayout(projectWidget);
        projectLayout->setContentsMargins(0, 0, 0, 0);

        QPushButton* toggleButton = new QPushButton;
        toggleButton->setIcon(QIcon(":/icons/icons/folder_closed.png"));
        toggleButton->setIconSize(QSize(16, 16));
        toggleButton->setFixedSize(24, 24);
        toggleButton->setFlat(true);
        toggleButton->setCursor(Qt::PointingHandCursor);
        toggleButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");
        QLabel* titleLabel = new QLabel("<b>" + projectName + "</b>");
        QHBoxLayout* headerLayout = new QHBoxLayout;
        headerLayout->addWidget(toggleButton);
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        QWidget* headerWidget = new QWidget;
        headerWidget->setLayout(headerLayout);
        projectLayout->addWidget(headerWidget);

        QWidget* ticketsContainer = new QWidget;
        QVBoxLayout* ticketsLayout = new QVBoxLayout(ticketsContainer);
        ticketsLayout->setContentsMargins(15, 5, 5, 5);
        ticketsContainer->setVisible(false);

        QSqlQuery ticketQuery;
        QString ticketSql = loadSqlQuery(":/sql/sql/getTicketsByProject.sql");
        ticketQuery.prepare(ticketSql);
        ticketQuery.bindValue(":projectId", projectId);

        if (ticketQuery.exec()) {
            while (ticketQuery.next()) {
                int ticketId = ticketQuery.value("id").toInt();
                QString title = ticketQuery.value("title").toString();
                QString tracker = ticketQuery.value("tracker").toString();

                QLabel* ticketLabel = new QLabel(QString("#%1 | %2 [%3]").arg(ticketId).arg(title).arg(tracker));
                ticketLabel->setCursor(Qt::PointingHandCursor);
                ticketLabel->setStyleSheet("QLabel:hover { text-decoration: underline; }");
                ticketLabel->setProperty("ticketId", ticketId);
                ticketLabel->installEventFilter(this);
                ticketsLayout->addWidget(ticketLabel);
            }
        } else {
            ticketsLayout->addWidget(new QLabel("⚠ Ошибка загрузки тикетов"));
        }

        projectLayout->addWidget(ticketsContainer);

        connect(toggleButton, &QPushButton::clicked, this, [=]() mutable {
            bool isVisible = ticketsContainer->isVisible();
            ticketsContainer->setVisible(!isVisible);
            toggleButton->setIcon(QIcon(isVisible ? ":/icons/icons/folder_closed.png" : ":/icons/icons/folder_open.png"));
        });

        ui->projectsLayout->addWidget(projectWidget);
    }

    ui->projectsLayout->addStretch();
}


void MyProjectsTab::loadEmployees() {
    QLayoutItem* item;
    while ((item = ui->employeesLayout->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) delete w;
        delete item;
    }

    QSqlQuery empQuery;
    QString empSql = loadSqlQuery(":/sql/sql/getEmployeesByDepartment.sql");
    empQuery.prepare(empSql);
    empQuery.bindValue(":userId", userId);

    if (!empQuery.exec()) {
        ui->employeesLayout->addWidget(new QLabel("Ошибка загрузки сотрудников"));
        return;
    }

    if (!empQuery.next()) {
        ui->employeesLayout->addWidget(new QLabel("Сотрудники не найдены"));
        return;
    }

    QString deptName = empQuery.value("department_name").toString();
    QLabel* deptLabel = new QLabel("Отдел: <b>" + deptName + "</b>");
    ui->employeesLayout->addWidget(deptLabel);

    do {
        int empId = empQuery.value("id").toInt();
        QString fullName = empQuery.value("last_name").toString() + " " + empQuery.value("first_name").toString();
        QString role = empQuery.value("role").toString();

        QWidget* empWidget = new QWidget;
        QVBoxLayout* empVBox = new QVBoxLayout(empWidget);
        empVBox->setContentsMargins(0, 0, 0, 0);

        QWidget* headerWidget = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(headerWidget);

        QPushButton* toggleButton = new QPushButton;
        toggleButton->setIcon(QIcon(":/icons/icons/folder_closed.png"));
        toggleButton->setIconSize(QSize(16, 16));
        toggleButton->setFixedSize(24, 24);
        toggleButton->setFlat(true);
        toggleButton->setCursor(Qt::PointingHandCursor);
        toggleButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");

        QLabel* empLabel = new QLabel(QString("%1 (%2)").arg(fullName, role));

        QPushButton* editButton = new QPushButton;
        editButton->setIcon(QIcon(":/icons/icons/edit.png"));
        editButton->setIconSize(QSize(16, 16));
        editButton->setFixedSize(24, 24);
        editButton->setFlat(true);
        editButton->setCursor(Qt::PointingHandCursor);
        editButton->setStyleSheet("QPushButton { background-color: transparent; border: none; }");

        layout->addWidget(toggleButton);
        layout->addWidget(empLabel);
        layout->addStretch();
        layout->addWidget(editButton);

        empVBox->addWidget(headerWidget);

        QWidget* ticketList = new QWidget;
        QVBoxLayout* ticketLayout = new QVBoxLayout(ticketList);
        ticketLayout->setContentsMargins(15, 5, 5, 5);
        ticketList->setVisible(false);

        empVBox->addWidget(ticketList);

        connect(toggleButton, &QPushButton::clicked, this, [=]() mutable {
            bool isVisible = ticketList->isVisible();
            ticketList->setVisible(!isVisible);
            toggleButton->setIcon(QIcon(isVisible ? ":/icons/icons/folder_closed.png" : ":/icons/icons/folder_open.png"));

            if (!isVisible && ticketLayout->isEmpty()) {
                QSqlQuery tQuery;
                QString ticketSql = loadSqlQuery(":/sql/sql/getTicketsByEmployee.sql");
                tQuery.prepare(ticketSql);
                tQuery.bindValue(":userId", empId);

                if (tQuery.exec()) {
                    while (tQuery.next()) {
                        int ticketId = tQuery.value("id").toInt();
                        QString title = tQuery.value("title").toString();
                        QString tracker = tQuery.value("tracker").toString();

                        QLabel* ticketLabel = new QLabel(QString("#%1 | %2 [%3]").arg(ticketId).arg(title).arg(tracker));
                        ticketLabel->setCursor(Qt::PointingHandCursor);
                        ticketLabel->setStyleSheet("QLabel:hover { text-decoration: underline; }");
                        ticketLabel->setProperty("ticketId", ticketId);
                        ticketLabel->installEventFilter(this);
                        ticketLayout->addWidget(ticketLabel);
                    }
                } else {
                    ticketLayout->addWidget(new QLabel("⚠ Ошибка загрузки тикетов"));
                }
            }
        });

        connect(editButton, &QPushButton::clicked, this, [=]() {
            EditEmployeeDialog dialog(empId, this);
            dialog.exec();
            loadEmployees();
        });

        ui->employeesLayout->addWidget(empWidget);
    } while (empQuery.next());

    ui->employeesLayout->addStretch();
}



bool MyProjectsTab::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QLabel* label = qobject_cast<QLabel*>(obj);
        if (label && label->property("ticketId").isValid()) {
            int ticketId = label->property("ticketId").toInt();

            auto* card = new TicketCard(ticketId, userId, tabWidget, this);
            tabWidget->addTab(card, QString("Тикет #%1").arg(ticketId));
            tabWidget->setCurrentWidget(card);

            connect(card, &TicketCard::ticketUpdated, this, &MyProjectsTab::loadProjects);
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MyProjectsTab::onCreateProjectClicked() {
    CreateProjectDialog dialog(this);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        loadProjects();
    }
}

void MyProjectsTab::onDeleteProjectClicked() {
    bool ok;
    QString projectName = QInputDialog::getText(this, "Удаление проекта", "Введите название проекта для удаления:",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || projectName.trimmed().isEmpty()) return;

    QSqlQuery idQuery;
    QString idSql = loadSqlQuery(":/sql/sql/getProjectIdByName.sql");
    idQuery.prepare(idSql);
    idQuery.bindValue(":name", projectName.trimmed());

    int projectId = -1;
    if (idQuery.exec() && idQuery.next()) {
        projectId = idQuery.value("id").toInt();
    } else {
        QMessageBox::warning(this, "Ошибка", "Проект с таким названием не найден.");
        return;
    }

    QSqlQuery query;
    QString sql = loadSqlQuery(":/sql/sql/getTicketsByProject.sql");
    query.prepare(sql);
    query.bindValue(":projectId", projectId);

    QString ticketList;
    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString title = query.value("title").toString();
            QString tracker = query.value("tracker").toString();
            ticketList += QString("• #%1 | %2 [%3]\n").arg(id).arg(title).arg(tracker);
        }
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления",
                                                              QString("Вы уверены, что хотите удалить проект \"%1\"?\n\n"
                                                                      "Данному проекту принадлежат следующие тикеты:\n\n%2")
                                                                  .arg(projectName)
                                                                  .arg(ticketList.isEmpty() ? "(Нет тикетов)" : ticketList),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    QStringList deletionSteps = {
        ":/sql/sql/deleteTicketHistoryByProject.sql",
        ":/sql/sql/deleteTicketFilesByProject.sql",
        ":/sql/sql/deleteTicketsByProject.sql",
        ":/sql/sql/deleteProject.sql"
    };

    for (const QString& path : deletionSteps) {
        QSqlQuery delQuery;
        QString stepSql = loadSqlQuery(path);
        delQuery.prepare(stepSql);
        delQuery.bindValue(":projectId", projectId);
        if (!delQuery.exec()) {
            QMessageBox::critical(this, "Ошибка", "Ошибка при удалении:\n" + delQuery.lastError().text());
            return;
        }
    }

    loadProjects();
    emit ticketsInvalidated();
}
