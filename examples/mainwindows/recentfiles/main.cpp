#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow *mainWin = new MainWindow;
    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
    mainWin->show();
    return app.exec();
}
