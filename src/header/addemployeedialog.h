#pragma once

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;

class AddEmployeeDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddEmployeeDialog(int managerId, QWidget* parent = nullptr);

private slots:
    void onAddClicked();

private:
    int managerId;
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *middleNameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QComboBox *roleCombo;
    QPushButton *addButton;

    void insertEmployee(const QString& first, const QString& last, const QString& middle,
                        const QString& email, const QString& password, const QString& role);
};
