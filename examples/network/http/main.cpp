#include <QApplication>

#include "httpwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HttpWindow httpWin;
    app.setMainWidget(&httpWin);
    httpWin.show();
    return app.exec();
}
