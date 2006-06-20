/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qcoreapplication.h>
#include <QtTest/QtTest>
#include <qhostaddress.h>
#include <qplatformdefs.h>
#include <qdebug.h>
#if QT_VERSION >= 0x040200
#include <qhash.h>
#include <qbytearray.h>
#include <qdatastream.h>
#endif

//TESTED_CLASS=
//TESTED_FILES=network/qhostaddress.h network/qhostaddress.cpp

class tst_QHostAddress : public QObject
{
    Q_OBJECT

public:
    tst_QHostAddress();
    virtual ~tst_QHostAddress();


public slots:
    void init();
    void cleanup();
private slots:
    void constructor_QString_data();
    void constructor_QString();
    void setAddress_QString_data();
    void setAddress_QString();
    void specialAddresses_data();
    void specialAddresses();
    void compare_data();
    void compare();
    void assignment();
    void scopeId();
#if QT_VERSION >= 0x040200
    void hashKey();
    void streaming_data();
    void streaming();
#endif
};

tst_QHostAddress::tst_QHostAddress()
{
}

tst_QHostAddress::~tst_QHostAddress()
{
}

Q_DECLARE_METATYPE(QHostAddress)

void tst_QHostAddress::init()
{
    qRegisterMetaType<QHostAddress>("QHostAddress");
}

void tst_QHostAddress::cleanup()
{
    // No cleanup is required.
}

void tst_QHostAddress::constructor_QString_data()
{
    setAddress_QString_data();
}

void tst_QHostAddress::constructor_QString()
{
    QFETCH(QString, address);
    QFETCH(bool, ok);
    QFETCH(int, protocol);

    QHostAddress hostAddr(address);

    if (address == "0.0.0.0" || address == "::") {
        QVERIFY(ok);
    } else {
        QVERIFY(hostAddr.isNull() != ok);
    }

    if (ok)
        QTEST(hostAddr.toString(), "resAddr");

    if ( protocol == 4 ) {
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv4Protocol || hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv6Protocol );
    } else if ( protocol == 6 ) {
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv4Protocol && hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv6Protocol );
    } else {
        QVERIFY( hostAddr.isNull() );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
    }
}

void tst_QHostAddress::setAddress_QString_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<bool>("ok");
    QTest::addColumn<QString>("resAddr");
    QTest::addColumn<int>("protocol"); // 4: IPv4, 6: IPv6, other: undefined

    //next we fill it with data
    QTest::newRow("ip4_00")  << QString("127.0.0.1") << (bool)TRUE << QString("127.0.0.1") << 4;
    QTest::newRow("ip4_01")  << QString("255.3.2.1") << (bool)TRUE << QString("255.3.2.1") << 4;
    QTest::newRow("ip4_03")  << QString(" 255.3.2.1") << (bool)TRUE << QString("255.3.2.1") << 4;
    QTest::newRow("ip4_04")  << QString("255.3.2.1\r ") << (bool)TRUE << QString("255.3.2.1") << 4;
    QTest::newRow("ip4_05")  << QString("0.0.0.0") << (bool)TRUE << QString("0.0.0.0") << 4;

    // for the format of IPv6 addresses see also RFC 1884
    QTest::newRow("ip6_00")  << QString("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << (bool)TRUE << QString("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << 6;
    QTest::newRow("ip6_01")  << QString("1080:0000:0000:0000:0008:0800:200C:417A") << (bool)TRUE << QString("1080:0:0:0:8:800:200C:417A") << 6;
    QTest::newRow("ip6_02")  << QString("1080:0:0:0:8:800:200C:417A") << (bool)TRUE << QString("1080:0:0:0:8:800:200C:417A") << 6;
    QTest::newRow("ip6_03")  << QString("1080::8:800:200C:417A") << (bool)TRUE << QString("1080:0:0:0:8:800:200C:417A") << 6;
    QTest::newRow("ip6_04")  << QString("FF01::43") << (bool)TRUE << QString("FF01:0:0:0:0:0:0:43") << 6;
    QTest::newRow("ip6_05")  << QString("::1") << (bool)TRUE << QString("0:0:0:0:0:0:0:1") << 6;
    QTest::newRow("ip6_06")  << QString("1::") << (bool)TRUE << QString("1:0:0:0:0:0:0:0") << 6;
    QTest::newRow("ip6_07")  << QString("::") << (bool)TRUE << QString("0:0:0:0:0:0:0:0") << 6;
    QTest::newRow("ip6_08")  << QString("0:0:0:0:0:0:13.1.68.3") << (bool)TRUE << QString("0:0:0:0:0:0:D01:4403") << 6;
    QTest::newRow("ip6_09")  << QString("::13.1.68.3") << (bool)TRUE <<  QString("0:0:0:0:0:0:D01:4403") << 6;
    QTest::newRow("ip6_10")  << QString("0:0:0:0:0:FFFF:129.144.52.38") << (bool)TRUE << QString("0:0:0:0:0:FFFF:8190:3426") << 6;
    QTest::newRow("ip6_11")  << QString("::FFFF:129.144.52.38") << (bool)TRUE << QString("0:0:0:0:0:FFFF:8190:3426") << 6;

    QTest::newRow("error_00")  << QString("foobarcom") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_01")  << QString("foo.bar.com") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_02")  << QString("") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_03")  << QString() << (bool)FALSE << QString() << 0;
    QTest::newRow("error_04")  << QString(" \t\r") << (bool)FALSE << QString() << 0;

    QTest::newRow("error_ip4_00")  << QString("256.9.9.9") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip4_01")  << QString("-1.9.9.9") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip4_02")  << QString("123.0.0") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip4_03")  << QString("123.0.0.0.0") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip4_04")  << QString("255.2 3.2.1") << (bool)FALSE << QString() << 0;

    QTest::newRow("error_ip6_00")  << QString(":") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_01")  << QString(":::") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_02")  << QString("::AAAA:") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_03")  << QString(":AAAA::") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_04")  << QString("FFFF:::129.144.52.38") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_05")  << QString("FEDC:BA98:7654:3210:FEDC:BA98:7654:3210:1234") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_06")  << QString("129.144.52.38::") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_07")  << QString("::129.144.52.38:129.144.52.38") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_08")  << QString(":::129.144.52.38") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_09")  << QString("1FEDC:BA98:7654:3210:FEDC:BA98:7654:3210") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_10")  << QString("::FFFFFFFF") << (bool)FALSE << QString() << 0;
    QTest::newRow("error_ip6_11")  << QString("::EFGH") << (bool)FALSE << QString() << 0;
}

void tst_QHostAddress::setAddress_QString()
{
    QFETCH(QString, address);
    QFETCH(bool, ok);
    QFETCH(int, protocol);

    QHostAddress hostAddr;
    QVERIFY(hostAddr.setAddress(address) == ok);

    if (ok)
        QTEST(hostAddr.toString(), "resAddr");

    if ( protocol == 4 ) {
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv4Protocol || hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv6Protocol );
    } else if ( protocol == 6 ) {
        QVERIFY( hostAddr.protocol() != QAbstractSocket::IPv4Protocol && hostAddr.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::IPv6Protocol );
    } else {
        QVERIFY( hostAddr.isNull() );
        QVERIFY( hostAddr.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol );
    }
}

void tst_QHostAddress::specialAddresses_data()
{
    QTest::addColumn<QString>("text");
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("result");

    QTest::newRow("localhost_1") << QString("127.0.0.1") << (int)QHostAddress::LocalHost << true;
    QTest::newRow("localhost_2") << QString("127.0.0.2") << (int)QHostAddress::LocalHost << false;
    QTest::newRow("localhost_3") << QString("127.0.0.2") << (int)QHostAddress::LocalHostIPv6 << false;

    QTest::newRow("localhost_ipv6_4") << QString("::1") << (int)QHostAddress::LocalHostIPv6 << true;
    QTest::newRow("localhost_ipv6_5") << QString("::2") << (int)QHostAddress::LocalHostIPv6 << false;
    QTest::newRow("localhost_ipv6_6") << QString("::1") << (int)QHostAddress::LocalHost << false;

    QTest::newRow("null_1") << QString("") << (int)QHostAddress::Null << true;
    QTest::newRow("null_2") << QString("bjarne") << (int)QHostAddress::Null << true;
    
    QTest::newRow("compare_from_null") << QString("") << (int)QHostAddress::Broadcast << false;

    QTest::newRow("broadcast_1") << QString("255.255.255.255") << (int)QHostAddress::Any << false;
    QTest::newRow("broadcast_2") << QString("255.255.255.255") << (int)QHostAddress::Broadcast << true;

    QTest::newRow("any_ipv6") << QString("::") << (int)QHostAddress::AnyIPv6 << true;
    QTest::newRow("any_ipv4") << QString("0.0.0.0") << (int)QHostAddress::Any << true;
}


void tst_QHostAddress::specialAddresses()
{
    QFETCH(QString, text);
    QFETCH(int, address);
    QFETCH(bool, result);
    QVERIFY((QHostAddress(text) == (QHostAddress::SpecialAddress)address) == result);

    QHostAddress setter;
    setter.setAddress(text);
    if (result) {
        QVERIFY(setter == (QHostAddress::SpecialAddress) address);
    } else {
#if QT_VERSION >= 0x040100
        QVERIFY(!((QHostAddress::SpecialAddress) address == setter));
#else
        QVERIFY(!(setter == (QHostAddress::SpecialAddress) address));
#endif
    }
}


void tst_QHostAddress::compare_data()
{
    QTest::addColumn<QHostAddress>("first");
    QTest::addColumn<QHostAddress>("second");
    QTest::addColumn<bool>("result");

    QTest::newRow("1") << QHostAddress() << QHostAddress() << true;
    QTest::newRow("2") << QHostAddress(QHostAddress::Any) << QHostAddress(QHostAddress::Any) << true;
    QTest::newRow("3") << QHostAddress(QHostAddress::AnyIPv6) << QHostAddress(QHostAddress::AnyIPv6) << true;
    QTest::newRow("4") << QHostAddress(QHostAddress::Broadcast) << QHostAddress(QHostAddress::Broadcast) << true;
    QTest::newRow("5") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::Broadcast) << false;
    QTest::newRow("6") << QHostAddress(QHostAddress::LocalHost) << QHostAddress(QHostAddress::LocalHostIPv6) << false;
    QTest::newRow("7") << QHostAddress() << QHostAddress(QHostAddress::LocalHostIPv6) << false;
}

void tst_QHostAddress::compare()
{
    QFETCH(QHostAddress, first);
    QFETCH(QHostAddress, second);
    QFETCH(bool, result);

    QCOMPARE(first == second, result);
}

void tst_QHostAddress::assignment()
{
#if QT_VERSION < 0x040100
    QSKIP("The tested assignment operators were added in Qt 4.1", SkipAll);
#else
    QHostAddress address;
    address = "127.0.0.1";
    QCOMPARE(address, QHostAddress("127.0.0.1"));

    address = "::1";
    QCOMPARE(address, QHostAddress("::1"));

    QHostAddress addr("4.2.2.1");
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = htonl(addr.toIPv4Address());
    address.setAddress((sockaddr *)&sockAddr);
    QCOMPARE(address, addr);
#endif
}

void tst_QHostAddress::scopeId()
{
#if QT_VERSION < 0x040100
    QSKIP("Scope ID support was added in Qt 4.1", SkipAll);
#else
    QHostAddress address("fe80::2e0:4cff:fefb:662a%eth0");
    QCOMPARE(address.scopeId(), QString("eth0"));
    QCOMPARE(address.toString().toLower(), QString("fe80:0:0:0:2e0:4cff:fefb:662a%eth0"));

    QHostAddress address2("fe80::2e0:4cff:fefb:662a");
    QCOMPARE(address2.scopeId(), QString());
    address2.setScopeId(QString("en0"));
    QCOMPARE(address2.toString().toLower(), QString("fe80:0:0:0:2e0:4cff:fefb:662a%en0"));

    address2 = address;
    QCOMPARE(address2.scopeId(), QString("eth0"));
    QCOMPARE(address2.toString().toLower(), QString("fe80:0:0:0:2e0:4cff:fefb:662a%eth0"));
#endif
}

#if QT_VERSION >= 0x040200
void tst_QHostAddress::hashKey()
{
    QHash<QHostAddress, QString> hostHash;
    hostHash.insert(QHostAddress(), "ole");
}

void tst_QHostAddress::streaming_data()
{
    QTest::addColumn<QHostAddress>("address");
    QTest::newRow("1") << QHostAddress();
    QTest::newRow("2") << QHostAddress(0xDEADBEEF);
    QTest::newRow("3") << QHostAddress("127.128.129.130");
    QTest::newRow("4") << QHostAddress("1080:0000:0000:0000:0008:0800:200C:417A");
    QTest::newRow("5") << QHostAddress("fe80::2e0:4cff:fefb:662a%eth0");
    QTest::newRow("6") << QHostAddress(QHostAddress::Null);
    QTest::newRow("7") << QHostAddress(QHostAddress::LocalHost);
    QTest::newRow("8") << QHostAddress(QHostAddress::LocalHostIPv6);
    QTest::newRow("9") << QHostAddress(QHostAddress::Broadcast);
    QTest::newRow("10") << QHostAddress(QHostAddress::Any);
    QTest::newRow("11") << QHostAddress(QHostAddress::AnyIPv6);
    QTest::newRow("12") << QHostAddress("foo.bar.com");
}

void tst_QHostAddress::streaming()
{
    QFETCH(QHostAddress, address);
    QByteArray ba;
    QDataStream ds1(&ba, QIODevice::WriteOnly);
    ds1 << address;
    QVERIFY(ds1.status() == QDataStream::Ok);
    QDataStream ds2(&ba, QIODevice::ReadOnly);
    QHostAddress address2;
    ds2 >> address2;
    QVERIFY(ds2.status() == QDataStream::Ok);
    QCOMPARE(address, address2);
}
#endif

QTEST_MAIN(tst_QHostAddress)
#include "tst_qhostaddress.moc"
