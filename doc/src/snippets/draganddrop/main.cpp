#include <QtGui>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow *window = new MainWindow;
    app.setMainWidget(window);
    window->show();

    return app.exec();
}
