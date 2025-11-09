#include "src/header/mainwindow.h"
#include "ui/ui_mainwindow.h"
#include "src/header/myticketstab.h"
#include "src/header/myprojectstab.h"
#include "src/header/profiletab.h"

#include <QTabBar>

MainWindow::MainWindow(int userId, const QString& name, const QString& role, QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentUserId(userId),
    currentUserName(name),
    currentUserRole(role)
{
    ui->setupUi(this);
    setWindowTitle("Тикет-система — " + currentUserName + " (" + currentUserRole + ")");
    setupTabs();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupTabs() {
    ui->tabWidget->setTabsClosable(true); // Включаем крестики глобально

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, [this](int index) {
        QWidget* tab = ui->tabWidget->widget(index);
        if (tab == ticketsTab || tab == projectsTab || tab == profileTab)
            return;

        ui->tabWidget->removeTab(index);
        delete tab;
    });

    ticketsTab = new MyTicketsTab(currentUserId, currentUserRole, ui->tabWidget, this);
    int ticketsIndex = ui->tabWidget->addTab(ticketsTab, "Мои тикеты");

    if (currentUserRole == "начальник" || currentUserRole == "работник") {
        projectsTab = new MyProjectsTab(currentUserId, currentUserRole, ui->tabWidget);
        int projectsIndex = ui->tabWidget->addTab(projectsTab, "Мои проекты");

        connect(projectsTab, &MyProjectsTab::ticketsInvalidated, ticketsTab, &MyTicketsTab::loadTickets);
        connect(ticketsTab, &MyTicketsTab::ticketsChanged, projectsTab, &MyProjectsTab::loadProjects);

        // Убираем крестик с вкладки "Мои проекты"
        ui->tabWidget->tabBar()->setTabButton(projectsIndex, QTabBar::RightSide, nullptr);
    }

    profileTab = new ProfileTab(currentUserId);
    int profileIndex = ui->tabWidget->addTab(profileTab, "Мой профиль");

    // Убираем крестик с вкладки "Мой профиль"
    ui->tabWidget->tabBar()->setTabButton(profileIndex, QTabBar::RightSide, nullptr);

    // Добавляем refresh-кнопку к "Мои тикеты"
    QToolButton* refreshButton = new QToolButton(this);
    refreshButton->setIcon(QIcon(":/icons/icons/refresh.png"));
    refreshButton->setToolTip("Обновить задачи");
    refreshButton->setAutoRaise(true);
    refreshButton->setIconSize(QSize(16, 16));

    ui->tabWidget->tabBar()->setTabButton(ticketsIndex, QTabBar::RightSide, refreshButton);
    connect(refreshButton, &QToolButton::clicked, ticketsTab, &MyTicketsTab::refreshTickets);

    // Убираем крестик с вкладки "Мои тикеты"
    ui->tabWidget->tabBar()->setTabButton(ticketsIndex, QTabBar::LeftSide, nullptr);  // только если крестик слева
}

