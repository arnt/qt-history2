#include <qcoreapplication.h>
#include <qdebug.h>

#include <QtTest/QtTest>
#include <QtDBus/QtDBus>

class MyObject: public QObject
{
    Q_OBJECT
public slots:
    void method(const QDBusMessage &msg) { serial = msg.serialNumber(); path = msg.path(); }

public:
    int serial;
    QString path;
    MyObject() : serial(0) { }
};

class tst_QDBusConnection: public QObject
{
    Q_OBJECT

private slots:
    void noConnection();
    void addConnection();
    void connect();
    void send();
    void sendAsync();
    void sendSignal();

    void registerObject();

public:
    bool callMethod(const QDBusConnection &conn, const QString &path);
};

class QDBusSpy: public QObject
{
    Q_OBJECT
public slots:
    void handlePing(const QString &str) { args.clear(); args << str; }
    void asyncReply(const QDBusMessage &msg) { args << msg; serial = msg.replySerialNumber(); }

public:
    QList<QVariant> args;
    int serial;
};

void tst_QDBusConnection::noConnection()
{
    QDBusConnection con = QDBusConnection::addConnection("unix:path=/dev/null", "testconnection");
    QVERIFY(!con.isConnected());

    // try sending a message. This should fail
    QDBusMessage msg = QDBusMessage::methodCall("org.kde.selftest", "/org/kde/selftest",
                                                "org.kde.selftest", "Ping");
    msg << QLatin1String("ping");

    QVERIFY(!con.send(msg));

    QDBusSpy spy;
    QVERIFY(con.sendWithReplyAsync(msg, &spy, SLOT(asyncReply)) == 0);

    QDBusMessage reply = con.sendWithReply(msg);
    QVERIFY(reply.type() == QDBusMessage::ErrorMessage);

    QDBusReply<void> voidreply(reply);
    QVERIFY(voidreply.isError());
    QVERIFY(!voidreply.isSuccess());

    QDBusConnection::closeConnection("testconnection");
}

void tst_QDBusConnection::sendSignal()
{
    QDBusConnection &con = QDBus::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::signal("/org/kde/selftest", "org.kde.selftest",
            "Ping");
    msg << QLatin1String("ping");

    QVERIFY(con.send(msg));

    QTest::qWait(1000);
}

void tst_QDBusConnection::send()
{
    QDBusConnection &con = QDBus::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::methodCall("org.freedesktop.DBus",
            "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");

    QDBusMessage reply = con.sendWithReply(msg);

    QCOMPARE(reply.count(), 1);
    QCOMPARE(reply.at(0).typeName(), "QStringList");
    QVERIFY(reply.at(0).toStringList().contains(con.baseService()));
}

void tst_QDBusConnection::sendAsync()
{
    QDBusConnection &con = QDBus::sessionBus();
    QVERIFY(con.isConnected());

    QDBusSpy spy;

    QDBusMessage msg = QDBusMessage::methodCall("org.freedesktop.DBus",
            "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    int msgId = con.sendWithReplyAsync(msg, &spy, SLOT(asyncReply(QDBusMessage)));
    QVERIFY(msgId != 0);

    QTest::qWait(1000);

    QCOMPARE(spy.args.value(0).typeName(), "QStringList");
    QVERIFY(spy.args.at(0).toStringList().contains(con.baseService()));
    QCOMPARE(spy.serial, msgId);
}

void tst_QDBusConnection::connect()
{
    QDBusSpy spy;

    QDBusConnection &con = QDBus::sessionBus();

    con.connect(con.baseService(), "/org/kde/selftest", "org.kde.selftest", "ping", &spy,
                 SLOT(handlePing(QString)));

    QDBusMessage msg = QDBusMessage::signal("/org/kde/selftest", "org.kde.selftest",
            "ping");
    msg << QLatin1String("ping");

    QVERIFY(con.send(msg));

    QTest::qWait(1000);

    QCOMPARE(spy.args.count(), 1);
    QCOMPARE(spy.args.at(0).toString(), QString("ping"));
}

void tst_QDBusConnection::addConnection()
{
    {
        QDBusConnection con = QDBusConnection::addConnection(
                QDBusConnection::SessionBus, "bubu");

        QVERIFY(con.isConnected());
        QVERIFY(!con.lastError().isValid());

        QDBusConnection con2("foo");
        QVERIFY(!con2.isConnected());
        QVERIFY(!con2.lastError().isValid());

        con2 = con;
        QVERIFY(con.isConnected());
        QVERIFY(con2.isConnected());
        QVERIFY(!con.lastError().isValid());
        QVERIFY(!con2.lastError().isValid());
    }

    {
        QDBusConnection con("bubu");
        QVERIFY(con.isConnected());
        QVERIFY(!con.lastError().isValid());
    }

    QDBusConnection::closeConnection("bubu");

    {
        QDBusConnection con("bubu");
        QVERIFY(!con.isConnected());
        QVERIFY(!con.lastError().isValid());
    }
}

void tst_QDBusConnection::registerObject()
{
    QDBusConnection &con = QDBus::sessionBus();
    QVERIFY(con.isConnected());

    // make sure nothing is using our paths:
    QVERIFY(!callMethod(con, "/"));
    QVERIFY(!callMethod(con, "/p1"));
    QVERIFY(!callMethod(con, "/p2"));
    QVERIFY(!callMethod(con, "/p1/q"));
    QVERIFY(!callMethod(con, "/p1/q/r"));

    {
        // register one object at root:
        MyObject obj;
        QVERIFY(con.registerObject("/", &obj, QDBusConnection::ExportSlots));
        QVERIFY(callMethod(con, "/"));
        QCOMPARE(obj.path, QString("/"));
    }
    // make sure it's gone
    QVERIFY(!callMethod(con, "/"));

    {
        // register one at an element:
        MyObject obj;
        QVERIFY(con.registerObject("/p1", &obj, QDBusConnection::ExportSlots));
        QVERIFY(!callMethod(con, "/"));
        QVERIFY(callMethod(con, "/p1"));
        QCOMPARE(obj.path, QString("/p1"));

        // re-register it somewhere else
        QVERIFY(con.registerObject("/p2", &obj, QDBusConnection::ExportSlots));
        QVERIFY(callMethod(con, "/p1"));
        QCOMPARE(obj.path, QString("/p1"));
        QVERIFY(callMethod(con, "/p2"));
        QCOMPARE(obj.path, QString("/p2"));
    }
    // make sure it's gone
    QVERIFY(!callMethod(con, "/p1"));
    QVERIFY(!callMethod(con, "/p2"));

    {
        // register at a deep path
        MyObject obj;
        QVERIFY(con.registerObject("/p1/q/r", &obj, QDBusConnection::ExportSlots));
        QVERIFY(!callMethod(con, "/"));
        QVERIFY(!callMethod(con, "/p1"));
        QVERIFY(!callMethod(con, "/p1/q"));
        QVERIFY(callMethod(con, "/p1/q/r"));
        QCOMPARE(obj.path, QString("/p1/q/r"));
    }
    // make sure it's gone
    QVERIFY(!callMethod(con, "/p1/q/r"));

    {
        MyObject obj;
        QVERIFY(con.registerObject("/p1/q2", &obj, QDBusConnection::ExportSlots));
        QVERIFY(callMethod(con, "/p1/q2"));
        QCOMPARE(obj.path, QString("/p1/q2"));

        // try unregistering
        con.unregisterObject("/p1/q2");
        QVERIFY(!callMethod(con, "/p1/q2"));

        // register it again
        QVERIFY(con.registerObject("/p1/q2", &obj, QDBusConnection::ExportSlots));
        QVERIFY(callMethod(con, "/p1/q2"));
        QCOMPARE(obj.path, QString("/p1/q2"));
        
        // now try removing things around it:
        con.unregisterObject("/p2");
        QVERIFY(callMethod(con, "/p1/q2")); // unrelated object shouldn't affect

        con.unregisterObject("/p1");
        QVERIFY(callMethod(con, "/p1/q2")); // unregistering just the parent shouldn't affect it

        con.unregisterObject("/p1/q2/r");
        QVERIFY(callMethod(con, "/p1/q2")); // unregistering non-existing child shouldn't affect it either

        con.unregisterObject("/p1/q");
        QVERIFY(callMethod(con, "/p1/q2")); // unregistering sibling (before) shouldn't affect

        con.unregisterObject("/p1/r");
        QVERIFY(callMethod(con, "/p1/q2")); // unregistering sibling (after) shouldn't affect

        // now remove it:
        con.unregisterObject("/p1", QDBusConnection::UnregisterTree);
        QVERIFY(!callMethod(con, "/p1/q2")); // we removed the full tree
    }
}

bool tst_QDBusConnection::callMethod(const QDBusConnection &conn, const QString &path)
{
    QDBusMessage msg = QDBusMessage::methodCall(conn.baseService(), path, "local.any", "method");
    QDBusMessage reply = conn.sendWithReply(msg, QDBusConnection::UseEventLoop);

    return reply.type() == QDBusMessage::ReplyMessage;
}    

QTEST_MAIN(tst_QDBusConnection)

#include "tst_qdbusconnection.moc"

