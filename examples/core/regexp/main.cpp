#include <QtGui>

#include "regexpwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    RegExpWindow window;
    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
