#include <QtGui>

#include "client.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Client client;
    app.setMainWidget(&client);
    client.show();
    return app.exec();
}
