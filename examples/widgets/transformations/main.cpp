#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *window = new MainWindow;
    window->show();
    app.setMainWidget(window);

    return app.exec();
}
