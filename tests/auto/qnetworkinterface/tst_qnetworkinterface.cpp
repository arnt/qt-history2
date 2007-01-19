/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION < 0x040200
QTEST_NOOP_MAIN
#else

#include <qcoreapplication.h>
#include <qnetworkinterface.h>
#include <qtcpsocket.h>

//TESTED_FILES=network/qnetworkinterface.cpp network/qnetworkinterface_p.h network/qnetworkinterface.h

class tst_QNetworkInterface : public QObject
{
    Q_OBJECT

public:
    tst_QNetworkInterface();
    virtual ~tst_QNetworkInterface();

private slots:
    void loopbackIPv4();
    void loopbackIPv6();
    void localAddress();
    void interfaceFromXXX();
    void copyInvalidInterface();
};

tst_QNetworkInterface::tst_QNetworkInterface()
{
}

tst_QNetworkInterface::~tst_QNetworkInterface()
{
}

void tst_QNetworkInterface::loopbackIPv4()
{
    QList<QHostAddress> all = QNetworkInterface::allAddresses();
    QVERIFY(all.contains(QHostAddress(QHostAddress::LocalHost)));
}

void tst_QNetworkInterface::loopbackIPv6()
{
    QList<QHostAddress> all = QNetworkInterface::allAddresses();

    bool loopbackfound = false;
    bool anyIPv6 = false;
    foreach (QHostAddress addr, all)
        if (addr == QHostAddress::LocalHostIPv6) {
            loopbackfound = true;
            anyIPv6 = true;
            break;
        } else if (addr.protocol() == QAbstractSocket::IPv6Protocol)
            anyIPv6 = true;
            
    QVERIFY(!anyIPv6 || loopbackfound);
}

void tst_QNetworkInterface::localAddress()
{
    QTcpSocket socket;
    socket.connectToHost("fluke.troll.no", 80);
    QVERIFY(socket.waitForConnected(5000));

    QHostAddress local = socket.localAddress();

    // make Apache happy on fluke
    socket.write("GET / HTTP/1.0\r\n\r\n");
    socket.waitForBytesWritten(1000);
    socket.close();

    // test that we can find the address that QTcpSocket reported
    QList<QHostAddress> all = QNetworkInterface::allAddresses();
    QVERIFY(all.contains(local));
}

void tst_QNetworkInterface::interfaceFromXXX()
{
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface iface, allInterfaces) {
        QVERIFY(QNetworkInterface::interfaceFromName(iface.name()).isValid());
        foreach (QNetworkAddressEntry entry, iface.addressEntries()) {
            if (entry.ip() == QHostAddress::LocalHost || entry.ip() == QHostAddress::LocalHostIPv6)
                QVERIFY(entry.broadcast().isNull());
            else
                QVERIFY(!entry.broadcast().isNull());
        }
    }
}

void tst_QNetworkInterface::copyInvalidInterface()
{
    // Force a copy of an interfaces that isn't likely to exist
    QNetworkInterface i = QNetworkInterface::interfaceFromName("plopp");
}

QTEST_MAIN(tst_QNetworkInterface)
#include "tst_qnetworkinterface.moc"

#endif
