#pragma once

#include <QDialog>

namespace Ui {
class UserProfileEditDialog;
}

class UserProfileEditDialog : public QDialog {
    Q_OBJECT

public:
    explicit UserProfileEditDialog(int userId, QWidget *parent = nullptr);
    ~UserProfileEditDialog();

signals:
    void profileUpdated();
    void loadEmployees();

private slots:
    void onSaveClicked();

private:
    Ui::UserProfileEditDialog *ui;
    int userId;

    bool validateInput(QString &errorMsg);
    bool updateUserInfo();
};
