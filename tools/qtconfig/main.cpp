#include <qapplication.h>
#include "mainwindow.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    MainWindow mw;
    app.setMainWidget(&mw);
    mw.show();
    return app.exec();
}
