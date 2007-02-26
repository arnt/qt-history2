#include <QtGui>

#include "mainwindow.h"

int main(int argv, char *args[])
{
    Q_INIT_RESOURCE(undoframework);

    QApplication app(argv, args);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
