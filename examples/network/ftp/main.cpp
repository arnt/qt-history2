#include <QtGui>

#include "ftp.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Ftp ftp;
    app.setMainWidget(&ftp);
    ftp.show();
    return app.exec();
}
