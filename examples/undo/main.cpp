#include <QApplication>
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow win;
    win.resize(800, 600);
    win.show();

    return app.exec();
};
