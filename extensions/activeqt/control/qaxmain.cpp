#include <qapplication.h>
#include <qaxfactory.h>

int main(int argc, char **argv)
{
    if (!QAxFactory::isServer())
	return -1;

    QApplication app(argc, argv);

    return app.exec();
}
