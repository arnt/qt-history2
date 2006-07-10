#include <qcoreapplication.h>
#include <qdebug.h>

#include <QtTest/QtTest>
#include <QtDBus/QtDBus>

class MyObject: public QObject
{
    Q_OBJECT
public slots:
    void method(const QDBusMessage &msg) { path = msg.path(); }

public:
    QString path;
    MyObject() { }
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

    void callSelf();

public:
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
    QDBusConnection con = QDBusConnection::addConnection("unix:path=/dev/null", "testconnection");
    QVERIFY(!con.isConnected());

    // try sending a message. This should fail
    QDBusMessage msg = QDBusMessage::methodCall("org.kde.selftest", "/org/kde/selftest",
                                                "org.kde.selftest", "Ping", con);
    msg << QLatin1String("ping");

    QVERIFY(!con.send(msg));

    QDBusSpy spy;
    QVERIFY(con.call(msg, &spy, SLOT(asyncReply)) == 0);

    QDBusMessage reply = con.call(msg);
    QVERIFY(reply.type() == QDBusMessage::ErrorMessage);

    QDBusReply<void> voidreply(reply);
    QVERIFY(!voidreply.isValid());

    QDBusConnection::closeConnection("testconnection");
}

void tst_QDBusConnection::sendSignal()
{
    QDBusConnection con = QDBus::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::signal("/org/kde/selftest", "org.kde.selftest",
                                            "Ping", con);
    msg << QLatin1String("ping");

    QVERIFY(con.send(msg));

    QTest::qWait(1000);
}

void tst_QDBusConnection::send()
{
    QDBusConnection con = QDBus::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::methodCall("org.freedesktop.DBus",
        "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames", con);

    QDBusMessage reply = con.call(msg);

    QCOMPARE(reply.count(), 1);
    QCOMPARE(reply.at(0).typeName(), "QStringList");
    QVERIFY(reply.at(0).toStringList().contains(con.baseService()));
}

void tst_QDBusConnection::sendAsync()
{
    QDBusConnection con = QDBus::sessionBus();
    QVERIFY(con.isConnected());

    QDBusSpy spy;

    QDBusMessage msg = QDBusMessage::methodCall("org.freedesktop.DBus",
            "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames", con);
    QVERIFY(con.call(msg, &spy, SLOT(asyncReply(QDBusMessage))));

    QTest::qWait(1000);

    QCOMPARE(spy.args.value(0).typeName(), "QStringList");
    QVERIFY(spy.args.at(0).toStringList().contains(con.baseService()));
}

void tst_QDBusConnection::connect()
{
    QDBusSpy spy;

    QDBusConnection con = QDBus::sessionBus();

    con.connect(con.baseService(), "/org/kde/selftest", "org.kde.selftest", "ping", &spy,
                 SLOT(handlePing(QString)));

    QDBusMessage msg = QDBusMessage::signal("/org/kde/selftest", "org.kde.selftest",
                                            "ping", con);
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
    QDBusConnection con = QDBus::sessionBus();
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
    QDBusMessage msg = QDBusMessage::methodCall(conn.baseService(), path, "local.any", "method", conn);
    QDBusMessage reply = conn.call(msg, QDBus::BlockWithGui);

    return reply.type() == QDBusMessage::ReplyMessage;
}    

class TestObject : public QObject
{
  Q_OBJECT
public:
  TestObject(QObject *parent = 0) : QObject(parent) {}
  ~TestObject() {}

public slots:
  void test0() { qDebug() << "test0"; }
  void test1(int i) { qDebug() << "test1" << i; }
  int test2() { qDebug() << "test2"; return 1; }
};

void tst_QDBusConnection::callSelf()
{
  TestObject testObject;
  QDBusConnection connection = QDBus::sessionBus();
  QVERIFY(connection.registerObject("/test", &testObject,
			 	    QDBusConnection::ExportContents
				    |QDBusConnection::ExportNonScriptableContents));
  QVERIFY(connection.registerService("com.trolltech.Qt.Autotests.ToSelf"));
  QDBusInterface interface("com.trolltech.Qt.Autotests.ToSelf","/test");
  QVERIFY(interface.isValid());
  
  interface.call(QDBus::Block, "test0");
  interface.call(QDBus::Block, "test1", 0);
  interface.call(QDBus::Block, "test2");
}

QTEST_MAIN(tst_QDBusConnection)

#include "tst_qdbusconnection.moc"

