#include "src/header/createprojectdialog.h"
#include "ui/ui_createprojectdialog.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>

CreateProjectDialog::CreateProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateProjectDialog) {
    ui->setupUi(this);
}

CreateProjectDialog::~CreateProjectDialog() {
    delete ui;
}

void CreateProjectDialog::on_saveButton_clicked() {
    QString name = ui->titleEdit->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название проекта не может быть пустым.");
        return;
    }

    int departmentId = 1;  // временно жёстко задано

    QSqlQuery query;
    query.prepare("INSERT INTO projects (name, department_id) VALUES (:name, :department_id)");
    query.bindValue(":name", name);
    query.bindValue(":department_id", departmentId);

    if (!query.exec()) {
        QString error = query.lastError().text();
        if (error.contains("Duplicate entry") && error.contains("projects.name")) {
            QMessageBox::warning(this, "Проект уже существует",
                                 "Проект с таким названием уже существует.\nПожалуйста, выберите другое имя.");
        } else {
            QMessageBox::critical(this, "Ошибка",
                                  "Не удалось сохранить проект:\n" + error);
        }
        return;
    }

    accept();
}

