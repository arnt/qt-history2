#include <QApplication>

#include "window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
