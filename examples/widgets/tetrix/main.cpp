#include <QApplication>

#include <stdlib.h>
#include <time.h>

#include "tetrixwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TetrixWindow window;
    window.show();
    srand(time(0));
    return app.exec();
}
