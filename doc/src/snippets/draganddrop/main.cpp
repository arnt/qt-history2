#include <QtGui>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *window1 = new MainWindow;
    MainWindow *window2 = new MainWindow;
    app.setMainWidget(window1);
    window1->show();
    window2->show();

    return app.exec();
}
