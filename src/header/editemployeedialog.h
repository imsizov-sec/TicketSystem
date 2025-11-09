#ifndef EDITEMPLOYEEDIALOG_H
#define EDITEMPLOYEEDIALOG_H

#include <QDialog>

namespace Ui {
class EditEmployeeDialog;
}

class EditEmployeeDialog : public QDialog {
    Q_OBJECT

public:
    explicit EditEmployeeDialog(int employeeId, QWidget *parent = nullptr);
    ~EditEmployeeDialog();

private slots:
    void onSaveClicked();

private:
    void loadEmployeeData();
    void updateEmployeeData();

    Ui::EditEmployeeDialog *ui;
    int employeeId;
};

#endif // EDITEMPLOYEEDIALOG_H
