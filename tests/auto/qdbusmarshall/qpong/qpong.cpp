#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

class Pong: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.selftest")
public slots:

    void ping(QDBusMessage msg)
    {
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

    if (!con.registerService("org.kde.selftest"))
        exit(2);

    Pong pong;
    con.registerObject("/org/kde/selftest", &pong, QDBusConnection::ExportAllSlots);

    printf("ready.\n");

    return app.exec();
}

#include "qpong.moc"
