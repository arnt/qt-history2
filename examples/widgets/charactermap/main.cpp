#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
