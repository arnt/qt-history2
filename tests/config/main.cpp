#include <qapplication.h>
#include "mainwindowimpl.h"


int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    MainWindow mw;
    app.setMainWidget(&mw);
    mw.show();
    return app.exec();
}
