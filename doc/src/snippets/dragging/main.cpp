#include <QtGui>

#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window *window = new Window;
    app.setMainWidget(window);
    window->show();

    return app.exec();
}
