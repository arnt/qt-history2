#include <QApplication>

#include "server.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server server;
    app.setMainWidget(&server);
    server.show();
    return app.exec();
}
