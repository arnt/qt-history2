#include <QApplication>

#include "controllerwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ControllerWindow controller;
    controller.show();
    return app.exec();
}
