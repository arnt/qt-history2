#include <qapplication.h>

#include "colorswatch.h"
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.connect(&app, SIGNAL(lastWindowClosed()), SLOT(quit()));

    MainWindow *mainwindow;
    ColorSwatch *colorswatch;

#if 0
    mainwindow = new MainWindow;
    mainwindow->setWindowTitle("Main Window");
    colorswatch = new ColorSwatch("White", mainwindow);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    mainwindow->show();
#endif

#if 0
    mainwindow = new MainWindow;
    mainwindow->setWindowTitle("Not Closable");
    colorswatch = new ColorSwatch("Red", mainwindow);
    colorswatch->setClosable(false);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    mainwindow->show();
#endif

#if 0
    mainwindow = new MainWindow;
    mainwindow->setWindowTitle("Not Movable");
    colorswatch = new ColorSwatch("Green", mainwindow);
    colorswatch->setMovable(false);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    mainwindow->show();
#endif

#if 0
    mainwindow = new MainWindow;
    mainwindow->setWindowTitle("Not Floatable");
    colorswatch = new ColorSwatch("Blue", mainwindow);
    colorswatch->setFloatable(false);
    colorswatch->setArea(Qt::DockWindowAreaLeft);
    mainwindow->show();
#endif

#if 1
    mainwindow = new MainWindow(0, Qt::WDestructiveClose);
    mainwindow->setWindowTitle("Allowed on Left and Right");
//     colorswatch = new ColorSwatch("Yellow", mainwindow);
//     colorswatch->setArea(Qt::DockWindowAreaLeft);
//     colorswatch->setAllowedAreas(Qt::DockWindowAreaLeft | Qt::DockWindowAreaRight);
    mainwindow->show();
#endif

#if 0
    mainwindow = new MainWindow;
    mainwindow->setWindowTitle("Allowed on Top and Bottom");
    colorswatch = new ColorSwatch("Magenta", mainwindow);
    colorswatch->setArea(Qt::DockWindowAreaTop);
    colorswatch->setAllowedAreas(Qt::DockWindowAreaTop | Qt::DockWindowAreaBottom);
    mainwindow->show();
#endif

    return app.exec();
}
