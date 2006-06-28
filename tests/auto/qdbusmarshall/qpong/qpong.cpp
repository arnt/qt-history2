#include <QtCore/QtCore>
#include <QtDBus/QtDBus>

class Pong: public QObject
{
    Q_OBJECT
public slots:

    void ping(QDBusMessage msg)
    {
        if (!msg.sendReply(msg.arguments()))
            exit(1);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QDBusConnection con = QDBus::sessionBus();
    if (!con.isConnected())
        exit(1);

    if (!con.registerService("org.kde.selftest"))
        exit(2);

    Pong pong;
    con.registerObject("/org/kde/selftest", &pong, QDBusConnection::ExportSlots);

    printf("ready.\n");

    return app.exec();
}

#include "qpong.moc"
