#include "Test.h"

#include <QCoreApplication>
#include <QStringList>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString arg;
    if (app.arguments().size() > 1)
        arg = app.arguments().at(1).toLower().trimmed();

    Test::Type type;
    if (arg == QLatin1String("qt4client"))
        type = Test::Qt4Client;
    else if (arg == QLatin1String("qt4server"))
        type = Test::Qt4Server;
    else if (arg == QLatin1String("qt3client"))
        type = Test::Qt3Client;
    else if (arg == QLatin1String("qt3server"))
        type = Test::Qt3Server;
    else {
        qDebug("usage: ./stressTest <qt3client|qt3server|qt4client|qt4server>");
        return 0;
    }

    Test test(type);
    
    return app.exec();
}
