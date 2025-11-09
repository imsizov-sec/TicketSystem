#include "src/header/userprofileeditdialog.h"
#include "ui/ui_userprofileeditdialog.h"
#include "utils/header/utils.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

UserProfileEditDialog::UserProfileEditDialog(int userId, QWidget *parent)
    : QDialog(parent), ui(new Ui::UserProfileEditDialog), userId(userId) {
    ui->setupUi(this);

    QSqlQuery query;
    query.prepare(loadSqlQuery(":/sql/sql/getUserInfoForEdit.sql"));
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        ui->firstNameEdit->setText(query.value("first_name").toString());
        ui->middleNameEdit->setText(query.value("middle_name").toString());
        ui->lastNameEdit->setText(query.value("last_name").toString());
        ui->emailEdit->setText(query.value("email").toString());
    }

    connect(ui->saveButton, &QPushButton::clicked, this, &UserProfileEditDialog::onSaveClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

UserProfileEditDialog::~UserProfileEditDialog() {
    delete ui;
}

void UserProfileEditDialog::onSaveClicked() {
    QString errorMsg;
    if (!validateInput(errorMsg)) {
        QMessageBox::warning(this, "Ошибка", errorMsg);
        return;
    }

    if (updateUserInfo()) {
        QMessageBox::information(this, "Успех", "Данные успешно обновлены.");
        emit profileUpdated();
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить изменения.");
    }
}

bool UserProfileEditDialog::validateInput(QString &errorMsg) {
    if (ui->firstNameEdit->text().trimmed().isEmpty() ||
        ui->lastNameEdit->text().trimmed().isEmpty() ||
        ui->emailEdit->text().trimmed().isEmpty()) {
        errorMsg = "Имя, фамилия и почта не могут быть пустыми.";
        return false;
    }

    if (!ui->emailEdit->text().contains('@')) {
        errorMsg = "Неверный формат почты. Адрес должен содержать символ '@'.";
        return false;
    }

    if (!ui->newPasswordEdit->text().isEmpty() || !ui->confirmPasswordEdit->text().isEmpty()) {
        if (ui->newPasswordEdit->text() != ui->confirmPasswordEdit->text()) {
            errorMsg = "Новые пароли не совпадают.";
            return false;
        }
        if (ui->newPasswordEdit->text().length() < 6) {
            errorMsg = "Новый пароль должен быть не короче 6 символов.";
            return false;
        }
    }

    return true;
}

bool UserProfileEditDialog::updateUserInfo() {
    // Проверка и обновление пароля
    if (!ui->newPasswordEdit->text().isEmpty()) {
        QSqlQuery check;
        check.prepare(loadSqlQuery(":/sql/sql/getUserPasswordHash.sql"));
        check.bindValue(":id", userId);
        if (!check.exec() || !check.next()) return false;

        QString currentHash = check.value(0).toString();
        QString inputHash = QString(QCryptographicHash::hash(ui->currentPasswordEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());

        if (currentHash != inputHash) {
            QMessageBox::warning(this, "Ошибка", "Неверный текущий пароль.");
            return false;
        }

        QString newHash = QString(QCryptographicHash::hash(ui->newPasswordEdit->text().toUtf8(), QCryptographicHash::Sha256).toHex());
        QSqlQuery updatePwd;
        updatePwd.prepare(loadSqlQuery(":/sql/sql/updateUserPassword.sql"));
        updatePwd.bindValue(":pwd", newHash);
        updatePwd.bindValue(":id", userId);
        if (!updatePwd.exec()) return false;
    }

    // Обновление имени, почты
    QSqlQuery query;
    query.prepare(loadSqlQuery(":/sql/sql/updateUserProfile.sql"));
    query.bindValue(":fn", ui->firstNameEdit->text().trimmed());
    query.bindValue(":mn", ui->middleNameEdit->text().trimmed());
    query.bindValue(":ln", ui->lastNameEdit->text().trimmed());
    query.bindValue(":email", ui->emailEdit->text().trimmed());
    query.bindValue(":id", userId);

    // return query.exec();
    if (!query.exec()) {
        return false;
    }
    return true;
}
