#include <QApplication>

#include "tetrixwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    TetrixWindow window;
    app.setMainWidget(&window);
    window.show();
    return app.exec();
}
