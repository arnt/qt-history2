#include <qapplication.h>
#include <qaxfactory.h>

int main(int argc, char **argv)
{
    QAxFactory::startServer();
    QApplication app(argc, argv);

    return app.exec();
}
