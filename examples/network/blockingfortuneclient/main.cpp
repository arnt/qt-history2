#include <QApplication>

#include "blockingclient.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    BlockingClient client;
    app.setMainWidget(&client);
    client.show();
    return app.exec();
}
