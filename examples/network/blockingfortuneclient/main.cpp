#include <QApplication>

#include "blockingclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    BlockingClient client;
    client.show();
    return client.exec();
}
