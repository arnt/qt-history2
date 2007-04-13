#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

static const char serviceName[] = "com.trolltech.autotests.qpong";
static const char objectPath[] = "/com/trolltech/qpong";
//static const char *interfaceName = serviceName;

class Pong: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.autotests.qpong")
public slots:

    void ping(QDBusMessage msg)
    {
        msg.setDelayedReply(true);
        if (!QDBusConnection::sessionBus().send(msg.createReply(msg.arguments())))
            exit(1);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDBusConnection con = QDBusConnection::sessionBus();
    if (!con.isConnected())
        exit(1);

    if (!con.registerService(serviceName))
        exit(2);

    Pong pong;
    con.registerObject(objectPath, &pong, QDBusConnection::ExportAllSlots);

    printf("ready.\n");

    return app.exec();
}

#include "qpong.moc"
