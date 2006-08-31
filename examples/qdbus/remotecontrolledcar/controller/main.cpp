#include <QtGui>
#include <QtDBus>

#include "controller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Controller controller;
    controller.show();
    return app.exec();
}
