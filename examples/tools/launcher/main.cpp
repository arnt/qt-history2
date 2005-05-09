#include <QtGui>

#include "launcher.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Launcher window;
    //window.resize(app.desktop()->size());
    window.show();
    return app.exec();
}
