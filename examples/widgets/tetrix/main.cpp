#include <QApplication>

#include <cstdlib>
#include <ctime>

#include "tetrixwindow.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    TetrixWindow window;
    app.setMainWidget(&window);
    window.show();
    srand(time(0));
    return app.exec();
}
