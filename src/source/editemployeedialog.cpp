#include "src/header/editemployeedialog.h"
#include "ui/ui_editemployeedialog.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

EditEmployeeDialog::EditEmployeeDialog(int employeeId_, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::EditEmployeeDialog),
    employeeId(employeeId_)
{
    ui->setupUi(this);
    setWindowTitle("Редактировать сотрудника");
    loadEmployeeData();

    connect(ui->saveButton, &QPushButton::clicked, this, &EditEmployeeDialog::onSaveClicked);
}

EditEmployeeDialog::~EditEmployeeDialog() {
    delete ui;
}

void EditEmployeeDialog::loadEmployeeData() {
    QSqlQuery query;
    query.prepare("SELECT first_name, last_name, middle_name, email, role FROM users WHERE id = :id");
    query.bindValue(":id", employeeId);

    if (query.exec() && query.next()) {
        ui->firstNameEdit->setText(query.value("first_name").toString());
        ui->lastNameEdit->setText(query.value("last_name").toString());
        ui->middleNameEdit->setText(query.value("middle_name").toString());
        ui->emailEdit->setText(query.value("email").toString());
        ui->roleCombo->setCurrentText(query.value("role").toString());
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные сотрудника.");
        reject();
    }
}

void EditEmployeeDialog::onSaveClicked() {
    if (!ui->emailEdit->text().contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный формат почты. Адрес должен содержать символ '@'.");
        return;
    }
    QSqlQuery query;
    query.prepare(R"(
        UPDATE users
        SET first_name = :first,
            last_name = :last,
            middle_name = :middle,
            email = :email,
            role = :role
        WHERE id = :id
    )");

    query.bindValue(":first", ui->firstNameEdit->text().trimmed());
    query.bindValue(":last", ui->lastNameEdit->text().trimmed());
    query.bindValue(":middle", ui->middleNameEdit->text().trimmed());
    query.bindValue(":email", ui->emailEdit->text().trimmed());
    query.bindValue(":role", ui->roleCombo->currentText());
    query.bindValue(":id", employeeId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить изменения: " + query.lastError().text());
        return;
    }

    accept();
}
