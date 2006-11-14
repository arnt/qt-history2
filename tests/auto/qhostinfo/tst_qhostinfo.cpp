/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// When using WinSock2 on Windows, it's the first thing that can be included
// (except qglobal.h), or else you'll get tons of compile errors
#include <qglobal.h>
#if defined(Q_OS_WIN) && !defined(QT_NO_GETADDRINFO)
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

#include <QtTest/QtTest>
#include <qcoreapplication.h>
#include <QDebug>
#include <QTcpSocket>
#include <private/qthread_p.h>
#include <QTcpServer>

#include <time.h>
#include <qlibrary.h>
#if defined Q_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <qhostinfo.h>

#if !defined(QT_NO_GETADDRINFO)
# include <sys/types.h>
# if defined(Q_OS_UNIX)
#  include <sys/socket.h>
# endif
# if !defined(Q_OS_WIN)
#  include <netdb.h>
# endif
#endif

//TESTED_FILES=qhostinfo.cpp qhostinfo.h qhostinfo_p.h qhostinfo_unix.cpp qhostinfo_win.cpp

const char * const lupinellaIp = "10.3.4.6";

class tst_QHostInfo : public QObject
{
    Q_OBJECT

public:
    tst_QHostInfo();
    virtual ~tst_QHostInfo();


public slots:
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void initTestCase();
    void lookupIPv4_data();
    void lookupIPv4();
    void lookupIPv6_data();
    void lookupIPv6();
    void reverseLookup_data();
    void reverseLookup();
    void idnatest();

    void blockingLookup_data();
    void blockingLookup();

    void raceCondition();
    void threadSafety();

protected slots:
    void resultsReady(const QHostInfo &);

private:
    bool ipv6LookupsAvailable;
    bool ipv6Available;
    bool lookupDone;
    QHostInfo lookupResults;
};

// Testing get/set functions
void tst_QHostInfo::getSetCheck()
{
    QHostInfo obj1;
    // HostInfoError QHostInfo::error()
    // void QHostInfo::setError(HostInfoError)
    obj1.setError(QHostInfo::HostInfoError(0));
    QCOMPARE(QHostInfo::HostInfoError(0), obj1.error());
    obj1.setError(QHostInfo::HostInfoError(1));
    QCOMPARE(QHostInfo::HostInfoError(1), obj1.error());

    // int QHostInfo::lookupId()
    // void QHostInfo::setLookupId(int)
    obj1.setLookupId(0);
    QCOMPARE(0, obj1.lookupId());
    obj1.setLookupId(INT_MIN);
    QCOMPARE(INT_MIN, obj1.lookupId());
    obj1.setLookupId(INT_MAX);
    QCOMPARE(INT_MAX, obj1.lookupId());
}

tst_QHostInfo::tst_QHostInfo()
{
}

tst_QHostInfo::~tst_QHostInfo()
{
}

void tst_QHostInfo::initTestCase()
{
    ipv6Available = false;
    ipv6LookupsAvailable = false;
#if !defined(QT_NO_GETADDRINFO)
    // check if the system getaddrinfo can do IPv6 lookups
    struct addrinfo hint, *result = 0;
    memset(&hint, 0, sizeof hint);
    hint.ai_family = AF_UNSPEC;

    int res = getaddrinfo("::1", "80", &hint, &result);
    if (res == 0) {
        // this test worked
        freeaddrinfo(result);
        res = getaddrinfo("ipv6-test.dev.troll.no", "80", &hint, &result);
        if (res == 0 && result != 0 && result->ai_family != AF_INET) {
            freeaddrinfo(result);
            ipv6LookupsAvailable = true;
        }
    }
#endif

    QTcpServer server;
    if (server.listen(QHostAddress("::1"))) {
        // We have IPv6 support
        ipv6Available = true;
    }
}

void tst_QHostInfo::init()
{
}

void tst_QHostInfo::cleanup()
{
}

void tst_QHostInfo::lookupIPv4_data()
{
    QTest::addColumn<QString>("hostname");
    QTest::addColumn<QString>("addresses");
    QTest::addColumn<int>("err");

    QTest::newRow("lupinella_00") << "l" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_01") << "lupinella" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_02") << "lupinella.troll.no" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_03") << "lupinella.trolltech.com" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("multiple_ip4") << "google.com" << "64.233.167.99 64.233.187.99 72.14.207.99" << int(QHostInfo::NoError);
    QTest::newRow("literal_ip4") << lupinellaIp << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("notfound") << "foo" << "" << int(QHostInfo::HostNotFound);
}

void tst_QHostInfo::lookupIPv4()
{
    QFETCH(QString, hostname);
    QFETCH(int, err);
    QFETCH(QString, addresses);

    lookupDone = false;
    QHostInfo::lookupHost(hostname, this, SLOT(resultsReady(const QHostInfo&)));

#if !defined QT_NO_THREAD
    QEventLoop eventLoop;
    eventLoop.exec();
    //QCoreApplication::instance()->enter_loop();
#endif

    int timeout = 30;
    while (!lookupDone && timeout) {
#if defined Q_OS_WIN32
        Sleep(1000);
#else
        timespec tm = {1, 0};
        nanosleep(&tm, 0);
#endif
        --timeout;
    }

    QCOMPARE((int)lookupResults.error(), (int)err);

    if (lookupDone) {
        QStringList tmp;
        for (int i = 0; i < lookupResults.addresses().count(); ++i)
            tmp.append(lookupResults.addresses().at(i).toString());
        tmp.sort();

        QStringList expected = addresses.split(' ');
        expected.sort();

        QCOMPARE(tmp.join(" "), expected.join(" "));
    } else {
        QFAIL("Timed out");
        // ###  how do we stop the pending lookup?
    }
}

void tst_QHostInfo::lookupIPv6_data()
{
    QTest::addColumn<QString>("hostname");
    QTest::addColumn<QString>("addresses");
    QTest::addColumn<int>("err");

    QTest::newRow("ip6") << "www.ipv6-net.org" << "62.93.217.177 2001:618:1401:0:0:0:0:4" << int(QHostInfo::NoError);
    QTest::newRow("literal_ip6") << "2001:6b0:1:ea:202:a5ff:fecd:13a6" << "2001:6b0:1:ea:202:a5ff:fecd:13a6" << int(QHostInfo::NoError);
    QTest::newRow("literal_shortip6") << "2001:618:1401::4" << "2001:618:1401:0:0:0:0:4" << int(QHostInfo::NoError);
}

void tst_QHostInfo::lookupIPv6()
{
    QFETCH(QString, hostname);
    QFETCH(int, err);
    QFETCH(QString, addresses);

    if (!ipv6LookupsAvailable)
        QSKIP("This platform does not support IPv6 lookups", SkipAll);

    lookupDone = false;
    QHostInfo::lookupHost(hostname, this, SLOT(resultsReady(const QHostInfo&)));

#if !defined QT_NO_THREAD
    QEventLoop eventLoop;
    eventLoop.exec();
    //QCoreApplication::instance()->enter_loop();
#endif

    int timeout = 30;
    while (!lookupDone && timeout) {
#if defined Q_OS_WIN32
        Sleep(1000);
#else
        timespec tm = {1, 0};
        nanosleep(&tm, 0);
#endif
        --timeout;
    }

    QCOMPARE((int)lookupResults.error(), (int)err);

    if (lookupDone) {
        QStringList tmp;
        for (int i = 0; i < lookupResults.addresses().count(); ++i)
            tmp.append(lookupResults.addresses().at(i).toString());
        tmp.sort();

        QStringList expected = addresses.split(' ');
        expected.sort();

        QCOMPARE(tmp.join(" ").toLower(), expected.join(" ").toLower());
    } else {
        QFAIL("Timed out");
        // ###  how do we stop the pending lookup?
    }
}

void tst_QHostInfo::reverseLookup_data()
{
    QTest::addColumn<QString>("address");
    QTest::addColumn<QStringList>("hostNames");
    QTest::addColumn<int>("err");

    QTest::newRow("trolltech.com") << QString("62.70.27.69") << QStringList(QString("diverse.troll.no")) << 0;
    QTest::newRow("www.nic.name") << QString("193.109.220.143") << (QStringList() << QString("www.nic.name") << QString("www.gnr.com") << QString("gnr.com") << QString("www.getyour.name") << QString("www.nameforlife.com") << QString("getyour.name") << QString("theglobalname.org") << QString("www.theglobalname.org")) << 0;

    QTest::newRow("classical.hexago.com") << QString("2001:5c0:0:2::24") << QStringList(QString("classical.hexago.com")) << 0;
    QTest::newRow("www.cisco.com") << QString("198.133.219.25") << QStringList(QString("www.cisco.com")) << 0;
    QTest::newRow("bogusexample.doenstexist.org") << QString("1::2::3::4") << QStringList() << 1;
}

void tst_QHostInfo::reverseLookup()
{
    QFETCH(QString, address);
    QFETCH(QStringList, hostNames);
    QFETCH(int, err);

    if (!ipv6LookupsAvailable && hostNames.contains("classical.hexago.com")) {
        QSKIP("IPv6 lookups are not supported on this platform", SkipSingle);
    }

    QHostInfo info = QHostInfo::fromName(address);

    if (err == 0) {
        QVERIFY(hostNames.contains(info.hostName()));
        QCOMPARE(info.addresses().first(), QHostAddress(address));
    } else {
        QCOMPARE(info.hostName(), address);
        QCOMPARE(info.error(), QHostInfo::HostNotFound);
    }

}

void tst_QHostInfo::idnatest()
{
    lookupDone = false;
    QHostInfo::lookupHost(QLatin1String("r\xe4ksm\xf6rg\xe5s.troll.no"), this, SLOT(resultsReady(const QHostInfo&)));

#if !defined QT_NO_THREAD
    QEventLoop eventLoop;
    eventLoop.exec();
    //QCoreApplication::instance()->enter_loop();
#endif

    int timeout = 30;
    while (!lookupDone && timeout) {
#if defined Q_OS_WIN32
        Sleep(1000);
#else
        timespec tm = {1, 0};
        nanosleep(&tm, 0);
#endif
        --timeout;
    }

    if (lookupDone) {
        QVERIFY(!lookupResults.addresses().isEmpty());

        QHostAddress result = lookupResults.addresses().first();

    } else {
        QFAIL("Timed out");
        // ###  how do we stop the pending lookup?
    }

}

void tst_QHostInfo::blockingLookup_data()
{
    QTest::addColumn<QString>("hostname");
    QTest::addColumn<QString>("addresses");
    QTest::addColumn<int>("err");

    QTest::newRow("lupinella_00") << "l" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_01") << "lupinella" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_02") << "lupinella.troll.no" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("lupinella_03") << "lupinella.trolltech.com" << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("multiple_ip4") << "google.com" << "64.233.167.99 64.233.187.99 72.14.207.99" << int(QHostInfo::NoError);
    QTest::newRow("literal_ip4") << lupinellaIp << lupinellaIp << int(QHostInfo::NoError);
    QTest::newRow("notfound") << "foo" << "" << int(QHostInfo::HostNotFound);
}

void tst_QHostInfo::blockingLookup()
{
    QFETCH(QString, hostname);
    QFETCH(int, err);
    QFETCH(QString, addresses);

    QHostInfo hostInfo = QHostInfo::fromName(hostname);
    QStringList tmp;
    for (int i = 0; i < hostInfo.addresses().count(); ++i)
        tmp.append(hostInfo.addresses().at(i).toString());
    tmp.sort();

    QCOMPARE((int)hostInfo.error(), (int)err);

    QStringList expected = addresses.split(' ');
    expected.sort();

    QCOMPARE(tmp.join(" "), expected.join(" "));
}

void tst_QHostInfo::raceCondition()
{
    for (int i = 0; i < 1000; ++i) {
        QTcpSocket socket;
        socket.connectToHost("notavalidname.troll.no", 80);
    }
}

class LookupThread : public QThread
{
protected:
    inline void run()
    {
         QHostInfo info = QHostInfo::fromName("www.trolltech.com");
         QCOMPARE(info.addresses().at(0).toString(), QString("62.70.27.70"));
    }
};

void tst_QHostInfo::threadSafety()
{
    const int nattempts = 5;
    LookupThread thr[nattempts];
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < nattempts; ++i)
            thr[i].start();
        for (int k = nattempts - 1; k >= 0; --k)
            thr[k].wait();
    }
}

void tst_QHostInfo::resultsReady(const QHostInfo &hi)
{
    lookupDone = true;
    lookupResults = hi;

#if !defined QT_NO_THREAD
    QThreadData *data = QThreadData::current();
    if(!data->eventLoops.isEmpty())
        data->eventLoops.top()->exit();
    //QCoreApplication::instance()->exit_loop();
#endif
}

QTEST_MAIN(tst_QHostInfo)
#include "tst_qhostinfo.moc"
