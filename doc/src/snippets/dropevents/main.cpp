#include <QtGui>

#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window *window = new Window;
    window->show();
    return app.exec();
}
