#include "src/header/addemployeedialog.h"
#include "utils/header/utils.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

AddEmployeeDialog::AddEmployeeDialog(int managerId_, QWidget* parent)
    : QDialog(parent), managerId(managerId_) {
    setWindowTitle("Добавить сотрудника");
    setMinimumSize(400, 300);

    firstNameEdit = new QLineEdit(this);
    lastNameEdit = new QLineEdit(this);
    middleNameEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);

    roleCombo = new QComboBox(this);
    roleCombo->addItems({"распределитель", "работник"});

    addButton = new QPushButton("Добавить", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Имя *", firstNameEdit);
    formLayout->addRow("Фамилия *", lastNameEdit);
    formLayout->addRow("Отчество", middleNameEdit);
    formLayout->addRow("Почта *", emailEdit);
    formLayout->addRow("Пароль *", passwordEdit);
    formLayout->addRow("Роль *", roleCombo);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(addButton);

    connect(addButton, &QPushButton::clicked, this, &AddEmployeeDialog::onAddClicked);
}

void AddEmployeeDialog::onAddClicked() {
    QString first = firstNameEdit->text().trimmed();
    QString last = lastNameEdit->text().trimmed();
    QString middle = middleNameEdit->text().trimmed();
    QString email = emailEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString role = roleCombo->currentText();

    if (first.isEmpty() || last.isEmpty() || email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните все обязательные поля.");
        return;
    }

    if (!email.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат почты. Адрес должен содержать символ '@'.");
        return;
    }

    QRegularExpression passwordRegex("^[A-Za-z0-9]{6,}$");
    if (!passwordRegex.match(password).hasMatch()) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен содержать только латинские буквы и цифры, без специальных символов, и быть длиной от 6 символов.");
        return;
    }

    insertEmployee(first, last, middle, email, password, role);
    accept();
}

void AddEmployeeDialog::insertEmployee(const QString& first, const QString& last, const QString& middle,
                                       const QString& email, const QString& password, const QString& role) {
    QSqlQuery deptQuery;
    QString deptSql = loadSqlQuery(":/sql/sql/getDepartmentIdByUserId.sql");
    deptQuery.prepare(deptSql);
    deptQuery.bindValue(":id", managerId);

    if (!deptQuery.exec() || !deptQuery.next()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось получить отдел начальника.");
        return;
    }

    int departmentId = deptQuery.value(0).toInt();
    QString fullName = last + " " + first + (middle.isEmpty() ? "" : (" " + middle));

    QSqlQuery insertQuery;
    QString insertSql = loadSqlQuery(":/sql/sql/insertUser.sql");
    insertQuery.prepare(insertSql);
    insertQuery.bindValue(":first", first);
    insertQuery.bindValue(":last", last);
    insertQuery.bindValue(":middle", middle.isEmpty() ? QVariant(QVariant::String) : middle);
    insertQuery.bindValue(":email", email);
    insertQuery.bindValue(":password", password);
    insertQuery.bindValue(":role", role);
    insertQuery.bindValue(":dept", departmentId);
    insertQuery.bindValue(":full", fullName);

    if (!insertQuery.exec()) {
        if (insertQuery.lastError().text().contains("Duplicate entry") &&
            insertQuery.lastError().text().contains("for key 'email'")) {
            QMessageBox::warning(this, "Ошибка", "Пользователь с такой почтой уже существует.");
        } else {
            QMessageBox::critical(this, "Ошибка", "Ошибка при добавлении сотрудника.");
        }
    } else {
        QMessageBox::information(this, "Успех", "Сотрудник успешно добавлен.");
    }
}
