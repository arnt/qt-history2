/* -*- C++ -*-
 */
#include <qcoreapplication.h>
#include <qmetatype.h>
#include <QtTest/QtTest>
#include <QtCore/qvariant.h>
#include <QtDBus/QtDBus>

#include "../qdbusmarshall/common.h"

Q_DECLARE_METATYPE(QVariantList)

#define TEST_INTERFACE_NAME "com.trolltech.QtDBus.MyObject"
#define TEST_SIGNAL_NAME "somethingHappened"

class MyObject: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.trolltech.QtDBus.MyObject")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.trolltech.QtDBus.MyObject\" >\n"
"    <property access=\"readwrite\" type=\"i\" name=\"prop1\" />\n"
"    <signal name=\"somethingHappened\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </signal>\n"
"    <method name=\"ping\" >\n"
"      <arg direction=\"in\" type=\"v\" name=\"ping\" />\n"
"      <arg direction=\"out\" type=\"v\" name=\"ping\" />\n"
"    </method>\n"
"    <method name=\"ping\" >\n"
"      <arg direction=\"in\" type=\"v\" name=\"ping1\" />\n"
"      <arg direction=\"in\" type=\"v\" name=\"ping2\" />\n"
"      <arg direction=\"out\" type=\"v\" name=\"pong1\" />\n"
"      <arg direction=\"out\" type=\"v\" name=\"pong2\" />\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    MyObject()
    {
        QObject *subObject = new QObject(this);
        subObject->setObjectName("subObject");
    }

public slots:

    void ping(const QDBusMessage &msg)
    {
        QDBusMessage reply = QDBusMessage::methodReply(msg);
        reply << static_cast<QList<QVariant> >(msg);
        if (!msg.connection().send(reply))
            exit(1);
    }
};

class Spy: public QObject
{
    Q_OBJECT
public:
    QString received;
    int count;

    Spy() : count(0)
    { }

public slots:
    void spySlot(const QString& arg)
    {
        received = arg;
        ++count;
    }
};

// helper function
void emitSignal(const QString &interface, const QString &name, const QString &arg)
{
    QDBusMessage msg = QDBusMessage::signal("/", interface, name);
    msg << arg;
    QDBus::sessionBus().send(msg);

    QTest::qWait(200);
}

class tst_QDBusInterface: public QObject
{
    Q_OBJECT
    MyObject obj;
private slots:
    void initTestCase();

    void introspect();

    void signal();
};

void tst_QDBusInterface::initTestCase()
{
    QDBusConnection &con = QDBus::sessionBus();
    QVERIFY(con.isConnected());

    con.registerObject("/", &obj, QDBusConnection::ExportAdaptors | QDBusConnection::ExportSlots |
                       QDBusConnection::ExportChildObjects);
}

void tst_QDBusInterface::introspect()
{
    QDBusConnection &con = QDBus::sessionBus();
    QDBusInterface *iface = con.findInterface(QDBus::sessionBus().baseService(), QLatin1String("/"),
                                              TEST_INTERFACE_NAME);

    const QMetaObject *mo = iface->metaObject();

    qDebug("Improve to a better testcase of QDBusMetaObject");
    QCOMPARE(mo->methodCount() - mo->methodOffset(), 3);
    QVERIFY(mo->indexOfSignal(TEST_SIGNAL_NAME "(QString)") != -1);

    QCOMPARE(mo->propertyCount() - mo->propertyOffset(), 1);
    QVERIFY(mo->indexOfProperty("prop1") != -1);

    iface->deleteLater();
}

void tst_QDBusInterface::signal()
{
    QDBusConnection &con = QDBus::sessionBus();
    QDBusInterface *iface = con.findInterface(con.baseService(), QLatin1String("/"),
                                              TEST_INTERFACE_NAME);

    QString arg = "So long and thanks for all the fish";
    {
        Spy spy;
        spy.connect(iface, SIGNAL(somethingHappened(QString)), SLOT(spySlot(QString)));

        emitSignal(TEST_INTERFACE_NAME, TEST_SIGNAL_NAME, arg);
        QCOMPARE(spy.count, 1);
        QCOMPARE(spy.received, arg);
    }

    iface->deleteLater();
}

QTEST_MAIN(tst_QDBusInterface)

#include "tst_qdbusinterface.moc"

