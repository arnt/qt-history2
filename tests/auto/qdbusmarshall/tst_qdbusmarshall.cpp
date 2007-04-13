#define DBUS_API_SUBJECT_TO_CHANGE
#include <QtCore/QtCore>
#include <QtTest/QtTest>
#include <QtDBus/QtDBus>

#include "common.h"
#include <limits>

static const char serviceName[] = "com.trolltech.autotests.qpong";
static const char objectPath[] = "/com/trolltech/qpong";
static const char *interfaceName = serviceName;

class tst_QDBusMarshall: public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();

private slots:
    void sendBasic_data();
    void sendBasic();

    void sendVariant_data();
    void sendVariant();

    void sendArrays_data();
    void sendArrays();

    void sendArrayOfArrays_data();
    void sendArrayOfArrays();

    void sendMaps_data();
    void sendMaps();

    void sendStructs_data();
    void sendStructs();

    void sendComplex_data();
    void sendComplex();

    void sendArgument_data();
    void sendArgument();

private:
    QProcess proc;
};

class WaitForQPong: public QObject
{
    Q_OBJECT
public:
    WaitForQPong();
    bool ok();
public Q_SLOTS:
    void ownerChange(const QString &name)
    {
        if (name == serviceName)
            loop.quit();
    }

private:
    QEventLoop loop;
};

WaitForQPong::WaitForQPong()
{
    QDBusConnection con = QDBusConnection::sessionBus();
    if (!ok()) {
        connect(con.interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                SLOT(ownerChange(QString)));
        QTimer::singleShot(2000, &loop, SLOT(quit()));
        loop.exec();
    }
}

bool WaitForQPong::ok()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName);
}

void tst_QDBusMarshall::initTestCase()
{
    commonInit();
#ifdef Q_OS_WIN
    proc.start("qpong");
#else
    proc.start("./qpong/qpong");
#endif
    QVERIFY(proc.waitForStarted());

    WaitForQPong w;
    QVERIFY(w.ok());
    //QTest::qWait(2000);
}

void tst_QDBusMarshall::cleanupTestCase()
{
    proc.close();
    proc.kill();
}

void tst_QDBusMarshall::sendBasic_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    // basic types:
    QTest::newRow("bool") << QVariant(false) << "b";
#if 1
    QTest::newRow("bool2") << QVariant(true) << "b";
    QTest::newRow("byte") << qVariantFromValue(uchar(1)) << "y";
    QTest::newRow("int16") << qVariantFromValue(short(2)) << "n";
    QTest::newRow("uint16") << qVariantFromValue(ushort(3)) << "q";
    QTest::newRow("int") << QVariant(1) << "i";
    QTest::newRow("uint") << QVariant(2U) << "u";
    QTest::newRow("int64") << QVariant(Q_INT64_C(3)) << "x";
    QTest::newRow("uint64") << QVariant(Q_UINT64_C(4)) << "t";
    QTest::newRow("double") << QVariant(42.5) << "d";
    QTest::newRow("string") << QVariant("ping") << "s";
    QTest::newRow("objectpath") << qVariantFromValue(QDBusObjectPath("/org/kde")) << "o";
    QTest::newRow("signature") << qVariantFromValue(QDBusSignature("g")) << "g";
    QTest::newRow("emptystring") << QVariant("") << "s";
    QTest::newRow("nullstring") << QVariant(QString()) << "s";
#endif
}

void tst_QDBusMarshall::sendVariant_data()
{
    sendBasic_data();

    // add a few more:
    QTest::newRow("variant") << qVariantFromValue(QDBusVariant(1)) << "v";

    QDBusVariant nested(1);
    QTest::newRow("variant-variant") << qVariantFromValue(QDBusVariant(qVariantFromValue(nested))) << "v";
}

void tst_QDBusMarshall::sendArrays_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    // arrays:
    QStringList strings;
    QTest::newRow("emptystringlist") << QVariant(strings) << "as";
    strings << "hello" << "world";
    QTest::newRow("stringlist") << QVariant(strings) << "as";

    strings.clear();
    strings << "" << "" << "";
    QTest::newRow("list-of-emptystrings") << QVariant(strings) << "as";

    strings.clear();
    strings << QString() << QString() << QString() << QString();
    QTest::newRow("list-of-nullstrings") << QVariant(strings) << "as";

    QByteArray bytearray;
    QTest::newRow("nullbytearray") << QVariant(bytearray) << "ay";
    bytearray = "";             // empty, not null
    QTest::newRow("emptybytearray") << QVariant(bytearray) << "ay";
    bytearray = "foo";
    QTest::newRow("bytearray") << QVariant(bytearray) << "ay";
    bytearray.clear();
    for (int i = 0; i < 4096; ++i)
        bytearray += QByteArray(1024, char(i));
    QTest::newRow("hugebytearray") << QVariant(bytearray) << "ay";

    QList<bool> bools;
    QTest::newRow("emptyboollist") << qVariantFromValue(bools) << "ab";
    bools << false << true << false;
    QTest::newRow("boollist") << qVariantFromValue(bools) << "ab";

    QList<short> shorts;
    QTest::newRow("emptyshortlist") << qVariantFromValue(shorts) << "an";
    shorts << 42 << -43 << 44 << 45 << -32768 << 32767;
    QTest::newRow("shortlist") << qVariantFromValue(shorts) << "an";

    QList<ushort> ushorts;
    QTest::newRow("emptyushortlist") << qVariantFromValue(ushorts) << "aq";
    ushorts << 12u << 13u << 14u << 15 << 65535;
    QTest::newRow("ushortlist") << qVariantFromValue(ushorts) << "aq";

    QList<int> ints;
    QTest::newRow("emptyintlist") << qVariantFromValue(ints) << "ai";
    ints << 42 << -43 << 44 << 45 << 2147483647 << -2147483647-1;
    QTest::newRow("intlist") << qVariantFromValue(ints) << "ai";

    QList<uint> uints;
    QTest::newRow("emptyuintlist") << qVariantFromValue(uints) << "au";
    uints << uint(12) << uint(13) << uint(14) << 4294967295U;
    QTest::newRow("uintlist") << qVariantFromValue(uints) << "au";

    QList<qlonglong> llints;
    QTest::newRow("emptyllintlist") << qVariantFromValue(llints) << "ax";
    llints << Q_INT64_C(99) << Q_INT64_C(-100)
           << Q_INT64_C(-9223372036854775807)-1 << Q_INT64_C(9223372036854775807);
    QTest::newRow("llintlist") << qVariantFromValue(llints) << "ax";

    QList<qulonglong> ullints;
    QTest::newRow("emptyullintlist") << qVariantFromValue(ullints) << "at";
    ullints << Q_UINT64_C(66) << Q_UINT64_C(67)
            << Q_UINT64_C(18446744073709551615);
    QTest::newRow("ullintlist") << qVariantFromValue(ullints) << "at";

    QList<double> doubles;
    QTest::newRow("emptydoublelist") << qVariantFromValue(doubles) << "ad";
    doubles << 1.2 << 2.2 << 4.4
            << -std::numeric_limits<double>::infinity()
            << std::numeric_limits<double>::infinity()
            << std::numeric_limits<double>::quiet_NaN();
    QTest::newRow("doublelist") << qVariantFromValue(doubles) << "ad";

    QVariantList variants;
    QTest::newRow("emptyvariantlist") << QVariant(variants) << "av";
    variants << QString("Hello") << QByteArray("World") << 42 << -43.0 << 44U << Q_INT64_C(-45)
             << Q_UINT64_C(46) << true << qVariantFromValue(short(-47))
             << qVariantFromValue(QDBusSignature("av"))
             << qVariantFromValue(QDBusVariant(qVariantFromValue(QDBusObjectPath("/"))));
    QTest::newRow("variantlist") << QVariant(variants) << "av";
}

void tst_QDBusMarshall::sendArrayOfArrays_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    // arrays:
    QList<QStringList> strings;
    QTest::newRow("empty-list-of-stringlist") << qVariantFromValue(strings) << "aas";
    strings << QStringList();
    QTest::newRow("list-of-emptystringlist") << qVariantFromValue(strings) << "aas";
    strings << (QStringList() << "hello" << "world")
            << (QStringList() << "hi" << "there")
            << (QStringList() << QString());
    QTest::newRow("stringlist") << qVariantFromValue(strings) << "aas";

    QList<QByteArray> bytearray;
    QTest::newRow("empty-list-of-bytearray") << qVariantFromValue(bytearray) << "aay";
    bytearray << QByteArray();
    QTest::newRow("list-of-emptybytearray") << qVariantFromValue(bytearray) << "aay";
    bytearray << "foo" << "bar" << "baz" << "" << QByteArray();
    QTest::newRow("bytearray") << qVariantFromValue(bytearray) << "aay";

    QList<QList<bool> > bools;
    QTest::newRow("empty-list-of-boollist") << qVariantFromValue(bools) << "aab";
    bools << QList<bool>();
    QTest::newRow("list-of-emptyboollist") << qVariantFromValue(bools) << "aab";
    bools << (QList<bool>() << false << true) << (QList<bool>() << false) << (QList<bool>());
    QTest::newRow("boollist") << qVariantFromValue(bools) << "aab";

    QList<QList<short> > shorts;
    QTest::newRow("empty-list-of-shortlist") << qVariantFromValue(shorts) << "aan";
    shorts << QList<short>();
    QTest::newRow("list-of-emptyshortlist") << qVariantFromValue(shorts) << "aan";
    shorts << (QList<short>() << 42 << -43 << 44 << 45)
           << (QList<short>() << -32768 << 32767)
           << (QList<short>());
    QTest::newRow("shortlist") << qVariantFromValue(shorts) << "aan";

    QList<QList<ushort> > ushorts;
    QTest::newRow("empty-list-of-ushortlist") << qVariantFromValue(ushorts) << "aaq";
    ushorts << QList<ushort>();
    QTest::newRow("list-of-emptyushortlist") << qVariantFromValue(ushorts) << "aaq";
    ushorts << (QList<ushort>() << 12u << 13u << 14u << 15)
            << (QList<ushort>() << 65535)
            << (QList<ushort>());
    QTest::newRow("ushortlist") << qVariantFromValue(ushorts) << "aaq";

    QList<QList<int> > ints;
    QTest::newRow("empty-list-of-intlist") << qVariantFromValue(ints) << "aai";
    ints << QList<int>();
    QTest::newRow("list-of-emptyintlist") << qVariantFromValue(ints) << "aai";
    ints << (QList<int>() << 42 << -43 << 44 << 45)
         << (QList<int>() << 2147483647 << -2147483647-1)
         << (QList<int>());
    QTest::newRow("intlist") << qVariantFromValue(ints) << "aai";

    QList<QList<uint> > uints;
    QTest::newRow("empty-list-of-uintlist") << qVariantFromValue(uints) << "aau";
    uints << QList<uint>();
    QTest::newRow("list-of-emptyuintlist") << qVariantFromValue(uints) << "aau";
    uints << (QList<uint>() << uint(12) << uint(13) << uint(14))
          << (QList<uint>() << 4294967295U)
          << (QList<uint>());
    QTest::newRow("uintlist") << qVariantFromValue(uints) << "aau";

    QList<QList<qlonglong> > llints;
    QTest::newRow("empty-list-of-llintlist") << qVariantFromValue(llints) << "aax";
    llints << QList<qlonglong>();
    QTest::newRow("list-of-emptyllintlist") << qVariantFromValue(llints) << "aax";
    llints << (QList<qlonglong>() << Q_INT64_C(99) << Q_INT64_C(-100))
           << (QList<qlonglong>() << Q_INT64_C(-9223372036854775807)-1 << Q_INT64_C(9223372036854775807))
           << (QList<qlonglong>());
    QTest::newRow("llintlist") << qVariantFromValue(llints) << "aax";

    QList<QList<qulonglong> > ullints;
    QTest::newRow("empty-list-of-ullintlist") << qVariantFromValue(ullints) << "aat";
    ullints << QList<qulonglong>();
    QTest::newRow("list-of-emptyullintlist") << qVariantFromValue(ullints) << "aat";
    ullints << (QList<qulonglong>() << Q_UINT64_C(66) << Q_UINT64_C(67))
            << (QList<qulonglong>() << Q_UINT64_C(18446744073709551615))
            << (QList<qulonglong>());
    QTest::newRow("ullintlist") << qVariantFromValue(ullints) << "aat";

    QList<QList<double> > doubles;
    QTest::newRow("empty-list-ofdoublelist") << qVariantFromValue(doubles) << "aad";
    doubles << QList<double>();
    QTest::newRow("list-of-emptydoublelist") << qVariantFromValue(doubles) << "aad";
    doubles << (QList<double>() << 1.2 << 2.2 << 4.4)
            << (QList<double>() << -std::numeric_limits<double>::infinity()
                << std::numeric_limits<double>::infinity()
                << std::numeric_limits<double>::quiet_NaN())
            << (QList<double>());
    QTest::newRow("doublelist") << qVariantFromValue(doubles) << "aad";

    QList<QVariantList> variants;
    QTest::newRow("emptyvariantlist") << qVariantFromValue(variants) << "aav";
    variants << QVariantList();
    QTest::newRow("emptyvariantlist") << qVariantFromValue(variants) << "aav";
    variants << (QVariantList() << QString("Hello") << QByteArray("World"))
             << (QVariantList() << 42 << -43.0 << 44U << Q_INT64_C(-45))
             << (QVariantList() << Q_UINT64_C(46) << true << qVariantFromValue(short(-47)));
    QTest::newRow("variantlist") << qVariantFromValue(variants) << "aav";
}

void tst_QDBusMarshall::sendMaps_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    QMap<int, QString> ismap;
    QTest::newRow("empty-is-map") << qVariantFromValue(ismap) << "a{is}";
    ismap[1] = "a";
    ismap[2000] = "b";
    ismap[-47] = "c";
    QTest::newRow("is-map") << qVariantFromValue(ismap) << "a{is}";

    QMap<QString, QString> ssmap;
    QTest::newRow("empty-ss-map") << qVariantFromValue(ssmap) << "a{ss}";
    ssmap["a"] = "a";
    ssmap["c"] = "b";
    ssmap["b"] = "c";
    QTest::newRow("ss-map") << qVariantFromValue(ssmap) << "a{ss}";

    QVariantMap svmap;
    QTest::newRow("empty-sv-map") << qVariantFromValue(svmap) << "a{sv}";
    svmap["a"] = 1;
    svmap["c"] = "b";
    svmap["b"] = QByteArray("c");
    svmap["d"] = 42U;
    svmap["e"] = qVariantFromValue(short(-47));
    svmap["f"] = qVariantFromValue(QDBusVariant(0));
    QTest::newRow("sv-map1") << qVariantFromValue(svmap) << "a{sv}";

    svmap.clear();
    svmap["ismap"] = qVariantFromValue(ismap);
    svmap["ssmap"] = qVariantFromValue(ssmap);
    QTest::newRow("sv-map2") << qVariantFromValue(svmap) << "a{sv}";
}

void tst_QDBusMarshall::sendStructs_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    QTest::newRow("point") << QVariant(QPoint(1, 2)) << "(ii)";
    QTest::newRow("pointf") << QVariant(QPointF(1.5, -1.5)) << "(dd)";

    QTest::newRow("size") << QVariant(QSize(1, 2)) << "(ii)";
    QTest::newRow("sizef") << QVariant(QSizeF(1.5, 1.5)) << "(dd)";

    QTest::newRow("rect") << QVariant(QRect(1, 2, 3, 4)) << "(iiii)";
    QTest::newRow("rectf") << QVariant(QRectF(0.5, 0.5, 1.5, 1.5)) << "(dddd)";

    QTest::newRow("line") << QVariant(QLine(1, 2, 3, 4)) << "((ii)(ii))";
    QTest::newRow("linef") << QVariant(QLineF(0.5, 0.5, 1.5, 1.5)) << "((dd)(dd))";

    QDate date(2006, 6, 18);
    QTime time(12, 25, 00);     // the date I wrote this test on :-)
    QTest::newRow("date") << QVariant(date) << "(iii)";
    QTest::newRow("time") << QVariant(time) << "(iiii)";
    QTest::newRow("datetime") << QVariant(QDateTime(date, time)) << "((iii)(iiii)i)";
}

void tst_QDBusMarshall::sendComplex_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    QList<QDateTime> dtlist;
    QTest::newRow("empty-datetimelist") << qVariantFromValue(dtlist) << "a((iii)(iiii)i)";
    dtlist << QDateTime();
    QTest::newRow("list-of-emptydatetime") << qVariantFromValue(dtlist) << "a((iii)(iiii)i)";
    dtlist << QDateTime::currentDateTime()
           << QDateTime(QDate(2006, 6, 18), QTime(13, 14, 00));
    QTest::newRow("datetimelist") << qVariantFromValue(dtlist) << "a((iii)(iiii)i)";

    QMap<qlonglong, QDateTime> lldtmap;
    QTest::newRow("empty-lldtmap") << qVariantFromValue(lldtmap) << "a{x((iii)(iiii)i)}";
    lldtmap[0] = QDateTime();
    lldtmap[1] = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 1), Qt::UTC);
    lldtmap[1150629776] = QDateTime(QDate(2006, 6, 18), QTime(11, 22, 56), Qt::UTC);
    QTest::newRow("lldtmap") << qVariantFromValue(lldtmap) << "a{x((iii)(iiii)i)}";

    QMap<int, QString> ismap;
    ismap[1] = "a";
    ismap[2000] = "b";
    ismap[-47] = "c";

    QMap<QString, QString> ssmap;
    ssmap["a"] = "a";
    ssmap["c"] = "b";
    ssmap["b"] = "c";

    QVariantMap svmap;
    svmap["a"] = 1;
    svmap["c"] = "b";
    svmap["b"] = QByteArray("c");
    svmap["d"] = 42U;
    svmap["e"] = qVariantFromValue(short(-47));
    svmap["f"] = qVariantFromValue(QDBusVariant(0));
    svmap["date"] = QDate::currentDate();
    svmap["time"] = QTime::currentTime();
    svmap["datetime"] = QDateTime::currentDateTime();
    svmap["pointf"] = QPointF(0.5, -0.5);
    svmap["ismap"] = qVariantFromValue(ismap);
    svmap["ssmap"] = qVariantFromValue(ssmap);
    svmap["dtlist"] = qVariantFromValue(dtlist);
    svmap["lldtmap"] = qVariantFromValue(lldtmap);
    QTest::newRow("sv-map") << qVariantFromValue(svmap) << "a{sv}";
}

void tst_QDBusMarshall::sendArgument_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<QString>("sig");

    QDBusArgument();
    QDBusArgument arg;

    arg = QDBusArgument();
    arg << true;
    QTest::newRow("bool") << qVariantFromValue(arg) << "b";

    arg = QDBusArgument();
    arg << false;
    QTest::newRow("bool2") << qVariantFromValue(arg) << "b";

    arg = QDBusArgument();
    arg << uchar(1);
    QTest::newRow("byte") << qVariantFromValue(arg) << "y";

    arg = QDBusArgument();
    arg << short(2);
    QTest::newRow("int16") << qVariantFromValue(arg) << "n";

    arg = QDBusArgument();
    arg << ushort(3);
    QTest::newRow("uint16") << qVariantFromValue(arg) << "q";

    arg = QDBusArgument();
    arg << 1;
    QTest::newRow("int32") << qVariantFromValue(arg) << "i";

    arg = QDBusArgument();
    arg << 2U;
    QTest::newRow("uint32") << qVariantFromValue(arg) << "u";

    arg = QDBusArgument();
    arg << Q_INT64_C(3);
    QTest::newRow("int64") << qVariantFromValue(arg) << "x";

    arg = QDBusArgument();
    arg << Q_UINT64_C(4);
    QTest::newRow("uint64") << qVariantFromValue(arg) << "t";

    arg = QDBusArgument();
    arg << 42.5;
    QTest::newRow("double") << qVariantFromValue(arg) << "d";

    arg = QDBusArgument();
    arg << QLatin1String("ping");
    QTest::newRow("string") << qVariantFromValue(arg) << "s";

    arg = QDBusArgument();
    arg << QDBusObjectPath("/org/kde");
    QTest::newRow("objectpath") << qVariantFromValue(arg) << "o";

    arg = QDBusArgument();
    arg << QDBusSignature("g");
    QTest::newRow("signature") << qVariantFromValue(arg) << "g";

    arg = QDBusArgument();
    arg << QLatin1String("");
    QTest::newRow("emptystring") << qVariantFromValue(arg) << "s";

    arg = QDBusArgument();
    arg << QString();
    QTest::newRow("nullstring") << qVariantFromValue(arg) << "s";

    arg = QDBusArgument();
    arg << QDBusVariant(1);
    QTest::newRow("variant") << qVariantFromValue(arg) << "v";

    arg = QDBusArgument();
    arg << QDBusVariant(qVariantFromValue(QDBusVariant(1)));
    QTest::newRow("variant-variant") << qVariantFromValue(arg) << "v";

    arg = QDBusArgument();
    arg.beginArray(QVariant::Int);
    arg << 1 << 2 << 3 << -4;
    arg.endArray();
    QTest::newRow("array-of-int") << qVariantFromValue(arg) << "ai";

    arg = QDBusArgument();
    arg.beginMap(QVariant::Int, QVariant::UInt);
    arg.beginMapEntry();
    arg << 1 << 2U;
    arg.endMapEntry();
    arg.beginMapEntry();
    arg << 3 << 4U;
    arg.endMapEntry();
    arg.endMap();
    QTest::newRow("map") << qVariantFromValue(arg) << "a{iu}";

    arg = QDBusArgument();
    arg.beginStructure();
    arg << 1 << 2U << short(-3) << ushort(4) << 5.0 << false;
    arg.endStructure();
    QTest::newRow("structure") << qVariantFromValue(arg) << "(iunqdb)";

#if 0
    // this is now unsupported
    arg << 1 << 2U << short(-3) << ushort(4) << 5.0 << false;
    QTest::newRow("many-args") << qVariantFromValue(arg) << "(iunqdb)iunqdb";
#endif
}

void tst_QDBusMarshall::sendBasic()
{
    QFETCH(QVariant, value);

    QDBusConnection con = QDBusConnection::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::createMethodCall(serviceName,
                                                      objectPath, interfaceName, "ping");
    msg << value;

    QDBusMessage reply = con.call(msg);
    //qDebug() << reply;

    QCOMPARE(reply.arguments().count(), msg.arguments().count());
    QTEST(reply.signature(), "sig");
    for (int i = 0; i < reply.arguments().count(); ++i)
        QVERIFY(compare(reply.arguments().at(i), msg.arguments().at(i)));
}

void tst_QDBusMarshall::sendVariant()
{
    QFETCH(QVariant, value);

    QDBusConnection con = QDBusConnection::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::createMethodCall(serviceName,
                                                      objectPath, interfaceName, "ping");
    msg << qVariantFromValue(QDBusVariant(value));

    QDBusMessage reply = con.call(msg);
 //   qDebug() << reply;

    QCOMPARE(reply.arguments().count(), msg.arguments().count());
    QCOMPARE(reply.signature(), QString("v"));
    for (int i = 0; i < reply.arguments().count(); ++i)
        QVERIFY(compare(reply.arguments().at(i), msg.arguments().at(i)));
}

void tst_QDBusMarshall::sendArrays()
{
    sendBasic();
}

void tst_QDBusMarshall::sendArrayOfArrays()
{
    sendBasic();
}

void tst_QDBusMarshall::sendMaps()
{
    sendBasic();
}

void tst_QDBusMarshall::sendStructs()
{
    sendBasic();
}

void tst_QDBusMarshall::sendComplex()
{
    sendBasic();
}

void tst_QDBusMarshall::sendArgument()
{
    QFETCH(QVariant, value);

    QDBusConnection con = QDBusConnection::sessionBus();

    QVERIFY(con.isConnected());

    QDBusMessage msg = QDBusMessage::createMethodCall(serviceName, objectPath,
                                                      interfaceName, "ping");
    msg << value;

    QDBusMessage reply = con.call(msg);
    //qDebug() << reply;

//    QCOMPARE(reply.arguments().count(), msg.arguments().count());
    QTEST(reply.signature(), "sig");
//    for (int i = 0; i < reply.arguments().count(); ++i)
//        QVERIFY(compare(reply.arguments().at(i), msg.arguments().at(i)));
}

QTEST_MAIN(tst_QDBusMarshall)
#include "tst_qdbusmarshall.moc"
