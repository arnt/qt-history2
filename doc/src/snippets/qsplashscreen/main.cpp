#include <QtGui>
#include <stdlib.h>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QPixmap pixmap(":/splash.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();

    sleep(5);
    QMainWindow window;
    window.show();
    splash.finish(&window);
    return app.exec();
}
