#include <qapplication.h>

#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.connect(&app, SIGNAL(lastWindowClosed()), SLOT(quit()));

    MainWindow *mainwindow = new MainWindow(0, Qt::WDestructiveClose);
    mainwindow->setWindowTitle("Qt Main Window Demo");
    mainwindow->show();

    return app.exec();
}
