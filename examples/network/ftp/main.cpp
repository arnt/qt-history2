#include <QApplication>

#include "ftpwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FtpWindow ftpWin;
    app.setMainWidget(&ftpWin);
    ftpWin.show();
    return app.exec();
}
