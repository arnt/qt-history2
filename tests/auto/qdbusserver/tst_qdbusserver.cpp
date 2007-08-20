#include <qcoreapplication.h>
#include <qdebug.h>

#include <QtTest/QtTest>
#include <QtDBus/QtDBus>

static const QString bus = "unix:path=/tmp/qdbus-test";
static const QString service = "com.trolltech.Qt.Autotests.QDBusServer";
static const QString path = "/com/trolltech/test";

class tst_QDBusServer : public QObject
{
    Q_OBJECT

    void connectToServer();
    void callMethod();
private slots:
};

void tst_QDBusServer::connectToServer()
{
    QDBusConnection connection = QDBusConnection::connectToBus(bus, "test-connection");
    QTest::qWait(100);
    QVERIFY(connection.isConnected());
}

void tst_QDBusServer::callMethod()
{
    QDBusConnection connection = QDBusConnection::connectToBus(bus, "test-connection");
    QTest::qWait(100);
    QVERIFY(connection.isConnected());
    //QDBusMessage msg = QDBusMessage::createMethodCall(bus, path, /*service*/"", "method");
    //QDBusMessage reply = connection.call(msg, QDBus::Block);
}

QTEST_MAIN(tst_QDBusServer)

#include "tst_qdbusserver.moc"
