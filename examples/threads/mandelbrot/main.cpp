#include <QApplication>

#include "mandelbrotwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MandelbrotWidget widget;
    app.setMainWidget(&widget);
    widget.show();
    return app.exec();
}
