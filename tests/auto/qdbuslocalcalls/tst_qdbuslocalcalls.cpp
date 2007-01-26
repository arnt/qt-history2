#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtTest>
#include <QtDBus>

Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QVector<int>)

class tst_QDBusLocalCalls: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "local.tst_QDBusLocalCalls")

    QDBusConnection conn;
public:
    tst_QDBusLocalCalls();

public Q_SLOTS:
    Q_SCRIPTABLE int echo(int value)
    { return value; }

    Q_SCRIPTABLE QString echo(const QString &value)
    { return value; }

    Q_SCRIPTABLE QDBusVariant echo(const QDBusVariant &value)
    { return value; }

    Q_SCRIPTABLE QVector<int> echo(const QVector<int> &value)
    { return value; }

    Q_SCRIPTABLE QString echo2(const QStringList &list, QString &out)
    { out = list[1]; return list[0]; }

    Q_SCRIPTABLE void delayed(const QDBusMessage &msg)
    { msg.setDelayedReply(true); }


private Q_SLOTS:
    void initTestCase();
    void makeInvalidCalls();
    void makeCalls_data();
    void makeCalls();
    void makeCallsVariant_data();
    void makeCallsVariant();
    void makeCallsTwoRets();
    void makeCallsComplex();
    void makeDelayedCalls();
};

tst_QDBusLocalCalls::tst_QDBusLocalCalls()
    : conn(QDBusConnection::sessionBus())
{
}

void tst_QDBusLocalCalls::initTestCase()
{
    QVERIFY(conn.isConnected());
    QVERIFY(conn.registerObject("/", this, QDBusConnection::ExportScriptableSlots));
}

void tst_QDBusLocalCalls::makeCalls_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::newRow("int") << QVariant(42);
    QTest::newRow("string") << QVariant("Hello, world");
}

void tst_QDBusLocalCalls::makeCallsVariant_data()
{
    makeCalls_data();
}

void tst_QDBusLocalCalls::makeInvalidCalls()
{
    {
        QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                              "/", QString(), "echo");
        QDBusMessage replyMsg = conn.call(callMsg);
        QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ErrorMessage));

        QDBusError error(replyMsg);
        QCOMPARE(int(error.type()), int(QDBusError::UnknownMethod));
    }

    {
        QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                              "/no_object", QString(), "echo");
        QDBusMessage replyMsg = conn.call(callMsg);
        QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ErrorMessage));

        QDBusError error(replyMsg);
        QCOMPARE(int(error.type()), int(QDBusError::UnknownMethod));
    }
}

void tst_QDBusLocalCalls::makeCalls()
{
    QFETCH(QVariant, value);
    QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                          "/", QString(), "echo");
    callMsg << value;
    QDBusMessage replyMsg = conn.call(callMsg);

    QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ReplyMessage));

    QVariantList replyArgs = replyMsg.arguments();
    QCOMPARE(replyArgs.count(), 1);
    QCOMPARE(replyArgs.at(0), value);
}

void tst_QDBusLocalCalls::makeCallsVariant()
{
    QFETCH(QVariant, value);
    QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                          "/", QString(), "echo");
    callMsg << qVariantFromValue(QDBusVariant(value));
    QDBusMessage replyMsg = conn.call(callMsg);

    QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ReplyMessage));

    QVariantList replyArgs = replyMsg.arguments();
    QCOMPARE(replyArgs.count(), 1);

    const QVariant &reply = replyArgs.at(0);
    QCOMPARE(reply.userType(), qMetaTypeId<QDBusVariant>());
    QCOMPARE(qvariant_cast<QDBusVariant>(reply).variant(), value);
}

void tst_QDBusLocalCalls::makeCallsTwoRets()
{
    QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                          "/", QString(), "echo2");
    callMsg << (QStringList() << "One" << "Two");
    QDBusMessage replyMsg = conn.call(callMsg);

    QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ReplyMessage));

    QVariantList replyArgs = replyMsg.arguments();
    QCOMPARE(replyArgs.count(), 2);
    QCOMPARE(replyArgs.at(0).toString(), QString::fromLatin1("One"));
    QCOMPARE(replyArgs.at(1).toString(), QString::fromLatin1("Two"));
}

void tst_QDBusLocalCalls::makeCallsComplex()
{
    qDBusRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QVector<int> >();

    QList<int> value;
    value << 1 << -42 << 47;
    QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                          "/", QString(), "echo");
    callMsg << qVariantFromValue(value);
    QDBusMessage replyMsg = conn.call(callMsg);

    QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ReplyMessage));

    QVariantList replyArgs = replyMsg.arguments();
    QCOMPARE(replyArgs.count(), 1);
    const QVariant &reply = replyArgs.at(0);
    QCOMPARE(reply.userType(), qMetaTypeId<QDBusArgument>());
    QCOMPARE(qdbus_cast<QList<int> >(reply), value);
}

void tst_QDBusLocalCalls::makeDelayedCalls()
{
    QDBusMessage callMsg = QDBusMessage::createMethodCall(conn.baseService(),
                                                          "/", QString(), "delayed");
    QTest::ignoreMessage(QtWarningMsg, "QDBusConnection: cannot call local method 'delayed' at object / (with signature '') on blocking mode");
    QDBusMessage replyMsg = conn.call(callMsg);
    QCOMPARE(int(replyMsg.type()), int(QDBusMessage::ErrorMessage));

    QDBusError error(replyMsg);
    QCOMPARE(int(error.type()), int(QDBusError::InternalError));
}

QTEST_MAIN(tst_QDBusLocalCalls)
#include "tst_qdbuslocalcalls.moc"
