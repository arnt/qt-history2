#include <QtGui>

#include "mainwindow.h"

int main(int argv, char *args[])
{
    QApplication app(argv, args);

    MainWindow mainWindow;
    mainWindow.resize(600, 400);
    mainWindow.show();

    return app.exec();
}
