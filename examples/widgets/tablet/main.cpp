#include <QtGui>

#include "mainwindow.h"
#include "tabletapplication.h"
#include "tabletcanvas.h"

int main(int argv, char *args[])
{
    TabletApplication app(argv, args);
    TabletCanvas *canvas = new TabletCanvas;
    app.setCanvas(canvas);

    MainWindow mainWindow(canvas);
    mainWindow.resize(500, 500);
    mainWindow.show();

    return app.exec();
}
