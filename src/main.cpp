#include <QApplication>
#include "src/header/loginwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QIcon appIcon(":/icons/icons/app_icon.png");
    app.setWindowIcon(appIcon);

    LoginWindow login;
    login.show();

    return app.exec();
}
