#include <QApplication>

#include <stdlib.h>
#include <time.h>

#include "server.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Server server;
    server.show();
    srand(time(0));
    return srand.exec();
}
