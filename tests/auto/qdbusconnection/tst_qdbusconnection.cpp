#include <qcoreapplication.h>
#include <qdebug.h>

#include <QtTest/QtTest>
#include <QtDBus/QtDBus>

class MyObject: public QObject
{
    Q_OBJECT
public slots:
    void method(const QDBusMessage &msg);

public:
    QString path;
    MyObject() { }
};

void MyObject::method(const QDBusMessage &msg)
{
    path = msg.path();
    //qDebug() << msg;
}

class tst_QDBusConnection: public QObject
{
    Q_OBJECT

    int signalsReceived;
public slots:
    void oneSlot() { ++signalsReceived; }

private slots:
    void noConnection();
    void connectToBus();
    void connect();
    void send();
    void sendAsync();
    void sendSignal();

    void registerObject_data();
    void registerObject();

    void callSelf();

    void slotsWithLessParameters();

public:
    QString serviceName() const { return "com.trolltech.Qt.Autotests.QDBusConnection"; }
    bool callMethod(const QDBusConnection &conn, const QString &path);
};

class QDBusSpy: public QObject
{
    Q_OBJECT
public slots:
    void handlePing(const QString &str) { args.clear(); args << str; }
    void asyncReply(const QDBusMessage &msg) { args = msg.arguments(); }

public:
    QList<QVariant> args;
};

void tst_QDBusConnection::noConnection()
{
    QDBusConnection con = QDBusConnection::connectToBus("unix:path=/dev/null", "testconnection");
    QVERIFY(!con.isConnected());

    // try sending a message. This should fail
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.selftest", "/org/kde/selftest",
                                                      "org.kde.selftest", "Ping");
    msg << QLatin1String("ping");

    QVERIFY(!con.send(msg));

    QDBusSpy spy;
    QVERIFY(con.callWithCallback(msg, &spy, SLOT(asyncReply)) == 0);

    QDBusMessage reply = con.call(msg);
    QVERIFY(reply.type() == QDBusMessage::ErrorMessage);

    QDBusReply<void> voidreply(reply);
    QVERIFY(!voidreply.isValid());

    QDBusConnection::disconnectFromBus("testconnection");
}

void tst_QDBusConnection::sendSignal()
{
    QDBusConnection con = QDBusConnection::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/selftest", "org.kde.selftest",
                                                  "Ping");
    msg << QLatin1String("ping");

    QVERIFY(con.send(msg));

    QTest::qWait(1000);
}

void tst_QDBusConnection::send()
{
    QDBusConnection con = QDBusConnection::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus",
        "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");

    QDBusMessage reply = con.call(msg);

    QCOMPARE(reply.arguments().count(), 1);
    QCOMPARE(reply.arguments().at(0).typeName(), "QStringList");
    QVERIFY(reply.arguments().at(0).toStringList().contains(con.baseService()));
}

void tst_QDBusConnection::sendAsync()
{
    QDBusConnection con = QDBusConnection::sessionBus();
    QVERIFY(con.isConnected());

    QDBusSpy spy;

    QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus",
            "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");
    QVERIFY(con.callWithCallback(msg, &spy, SLOT(asyncReply(QDBusMessage))));

    QTest::qWait(1000);

    QCOMPARE(spy.args.value(0).typeName(), "QStringList");
    QVERIFY(spy.args.at(0).toStringList().contains(con.baseService()));
}

void tst_QDBusConnection::connect()
{
    QDBusSpy spy;

    QDBusConnection con = QDBusConnection::sessionBus();

    con.connect(con.baseService(), "/org/kde/selftest", "org.kde.selftest", "ping", &spy,
                 SLOT(handlePing(QString)));

    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/selftest", "org.kde.selftest",
                                                  "ping");
    msg << QLatin1String("ping");

    QVERIFY(con.send(msg));

    QTest::qWait(1000);

    QCOMPARE(spy.args.count(), 1);
    QCOMPARE(spy.args.at(0).toString(), QString("ping"));
}

void tst_QDBusConnection::connectToBus()
{
    {
        QDBusConnection con = QDBusConnection::connectToBus(
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

    QDBusConnection::disconnectFromBus("bubu");

    {
        QDBusConnection con("bubu");
        QVERIFY(!con.isConnected());
        QVERIFY(!con.lastError().isValid());
    }
}

void tst_QDBusConnection::registerObject_data()
{
    QTest::addColumn<QString>("path");

    QTest::newRow("/") << "/";
    QTest::newRow("/p1") << "/p1";
    QTest::newRow("/p2") << "/p2";
    QTest::newRow("/p1/q") << "/p1/q";
    QTest::newRow("/p1/q/r") << "/p1/q/r";
}

void tst_QDBusConnection::registerObject()
{
    QFETCH(QString, path);

    QDBusConnection con = QDBusConnection::sessionBus();
    QVERIFY(con.isConnected());

#if 1
    //QVERIFY(!callMethod(con, path));
    {
        // register one object at root:
        MyObject obj;
        QVERIFY(con.registerObject(path, &obj, QDBusConnection::ExportAllSlots));
        QCOMPARE(con.objectRegisteredAt(path), &obj);
        QVERIFY(callMethod(con, path));
        QCOMPARE(obj.path, path);
    }
    // make sure it's gone
    QVERIFY(!callMethod(con, path));

#else

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
        qDebug() << obj.path;
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
        qDebug() << obj.path;
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
#endif
}

bool tst_QDBusConnection::callMethod(const QDBusConnection &conn, const QString &path)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(conn.baseService(), path, "", "method");
    QDBusMessage reply = conn.call(msg, QDBus::Block/*WithGui*/);
#if 0
    switch (reply.type()) {
        case QDBusMessage::InvalidMessage:
            qDebug() << "InvalidMessage:" << reply;
            break;
        case QDBusMessage::MethodCallMessage:
            qDebug() << "MethodCallMessage:" << reply;
            break;
        case QDBusMessage::ReplyMessage:
            qDebug() << "ReplyMessage:" << reply;
            break;
        case QDBusMessage::ErrorMessage:
            qDebug() << "ErrorMessage:" << reply;
            break;
        case QDBusMessage::SignalMessage:
            qDebug() << "SignalMessage:" << reply;
            break;
        default:
            break;
    }
#endif
    return reply.type() == QDBusMessage::ReplyMessage;
}

class TestObject : public QObject
{
Q_OBJECT
public:
    TestObject(QObject *parent = 0) : QObject(parent) {}
    ~TestObject() {}

    QString func;

public slots:
    void test0() { func = "test0"; }
    void test1(int i) { func = "test1 " + QString::number(i); }
    int test2() { func = "test2"; return 43; }
    int test3(int i) { func = "test2"; return i + 1; }
};

void tst_QDBusConnection::callSelf()
{
    TestObject testObject;
    QDBusConnection connection = QDBusConnection::sessionBus();
    QVERIFY(connection.registerObject("/test", &testObject,
            QDBusConnection::ExportAllContents));
    QCOMPARE(connection.objectRegisteredAt("/test"), &testObject);
    QVERIFY(connection.registerService(serviceName()));
    QDBusInterface interface(serviceName(), "/test");
    QVERIFY(interface.isValid());

    interface.call(QDBus::Block, "test0");
    QCOMPARE(testObject.func, QString("test0"));
    interface.call(QDBus::Block, "test1", 42);
    QCOMPARE(testObject.func, QString("test1 42"));
    QDBusMessage reply = interface.call(QDBus::Block, "test2");
    QCOMPARE(testObject.func, QString("test2"));
    QCOMPARE(reply.arguments().value(0).toInt(), 43);

    QDBusMessage msg = QDBusMessage::createMethodCall(serviceName(), "/test",
            serviceName(), "test3");
    msg << 44;
    reply = connection.call(msg);
    QCOMPARE(reply.arguments().value(0).toInt(), 45);
}

void tst_QDBusConnection::slotsWithLessParameters()
{
    QDBusConnection con = QDBusConnection::sessionBus();

    QDBusMessage signal = QDBusMessage::createSignal("/", "com.trolltech.TestCase",
                                                     "oneSignal");
    signal << "one parameter";

    signalsReceived = 0;
    QVERIFY(con.connect(con.baseService(), signal.path(), signal.interface(),
                        signal.member(), this, SLOT(oneSlot())));
    QVERIFY(con.send(signal));
    QTest::qWait(100);
    QCOMPARE(signalsReceived, 1);

    // disconnect and try with a signature
    signalsReceived = 0;
    QVERIFY(con.disconnect(con.baseService(), signal.path(), signal.interface(),
                           signal.member(), this, SLOT(oneSlot())));
    QVERIFY(con.connect(con.baseService(), signal.path(), signal.interface(),
                        signal.member(), "s", this, SLOT(oneSlot())));
    QVERIFY(con.send(signal));
    QTest::qWait(100);
    QCOMPARE(signalsReceived, 1);
}

QTEST_MAIN(tst_QDBusConnection)

#include "tst_qdbusconnection.moc"

