#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QDBusServer server("unix:path=/tmp/qdbus-test");
    return app.exec();
}
