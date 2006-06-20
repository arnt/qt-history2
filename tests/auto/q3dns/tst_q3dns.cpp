/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <q3dns.h>
#include <qapplication.h>
#include <q3socket.h>





//TESTED_CLASS=
//TESTED_FILES=compat/other/q3dns.h compat/other/q3dns.cpp

class tst_Q3Dns : public QObject
{
    Q_OBJECT

public:
    tst_Q3Dns();
    virtual ~tst_Q3Dns();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void destructor();
    void literals();
    void txtRecords();
    void longTxtRecord();
    void simpleLookup();

protected slots:
    void txtRecordAnswer();
    void longTxtRecordAnswer();
    void simpleLookupDone();
};

tst_Q3Dns::tst_Q3Dns()
{
}

tst_Q3Dns::~tst_Q3Dns()
{
}

void tst_Q3Dns::initTestCase()
{
}

void tst_Q3Dns::cleanupTestCase()
{
}

void tst_Q3Dns::init()
{
}

void tst_Q3Dns::cleanup()
{
}

void tst_Q3Dns::destructor()
{
    /*
    The following small program used to crash because of a bug in the Q3Dns
    constructor that should be fixed by change 67978:

	#include <qapplication.h>
	#include <qsocket.h>

	int main( int argc, char **argv )
	{
	    QApplication a( argc, argv );
	    Q3Socket *s = new Q3Socket( &a );
	    s->connectToHost( "ftp.trolltech.com", 21 );
	    return 0;
	}
    */
    int c = 0;
    char **v = 0;
    QCoreApplication a(c, v);
    Q3Socket *s = new Q3Socket(&a);
    s->connectToHost("ftp.trolltech.com", 21);

    // dummy verify since this test only makes shure that it does not crash
    QVERIFY( TRUE );
}

void tst_Q3Dns::literals()
{
    int c = 0;
    char **v = 0;
    QCoreApplication a(c, v);

    Q3Dns ip4literal1("4.2.2.1", Q3Dns::A);
    QCOMPARE((int) ip4literal1.addresses().count(), 1);
    QCOMPARE(ip4literal1.addresses().first().toString(), QString("4.2.2.1"));

    Q3Dns ip4literal2("4.2.2.1", Q3Dns::Aaaa);
    QCOMPARE((int) ip4literal2.addresses().count(), 0);

    Q3Dns ip6literal1("::1", Q3Dns::A);
    QCOMPARE((int) ip6literal1.addresses().count(), 0);

    Q3Dns ip6literal2("::1", Q3Dns::Aaaa);
    QCOMPARE(ip6literal2.addresses().first().toString(), QString("0:0:0:0:0:0:0:1"));
    QCOMPARE((int) ip6literal2.addresses().count(), 1);
}

void tst_Q3Dns::txtRecords()
{
    QSKIP("TXT record support is broken.", SkipAll);
    int argc = 0;
    char **argv = 0;
    QCoreApplication qapp(argc, argv);

    Q3Dns dns("Sales._ipp._tcp.dns-sd.org", Q3Dns::Txt);
    connect(&dns, SIGNAL(resultsReady()), SLOT(txtRecordAnswer()));
    QTestEventLoop::instance().enterLoop(10);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out while looking up TXT record for Sales._ipp._tcp.dns-sd.org");

    QStringList texts = dns.texts();
#if defined Q_OS_DARWIN
    QSKIP("TXT records in Q3Dns don't work for Mac OS X.", SkipSingle);
#endif
    QVERIFY(!texts.isEmpty());
    QCOMPARE(texts.at(0), QString("rp=lpt1"));
}

void tst_Q3Dns::txtRecordAnswer()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Dns::longTxtRecord()
{
    QSKIP("Long TXT records in Q3Dns don't work.", SkipSingle);

    int c = 0;
    char **v = 0;
    QCoreApplication a(c, v);

    Q3Dns dns(QString::fromLatin1("andreas.hanssen.name"), Q3Dns::Txt);
    QObject::connect(&dns, SIGNAL(resultsReady()), this, SLOT(longTxtRecordAnswer()));

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
	QFAIL("Network operation timed out");

    QStringList list = dns.texts();

    QCOMPARE(list.count(), 1);
    QCOMPARE(list[0], QString::fromLatin1("I have a remarkable solution to Fermat's last theorem, but it doesn't fit into this TXT record"));
}

void tst_Q3Dns::longTxtRecordAnswer()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Dns::simpleLookup()
{
    // Stuff
    int c = 0;
    char **v = 0;
    QCoreApplication a(c, v);
    Q3Dns dns("www.trolltech.com");

    QSignalSpy spy(&dns, SIGNAL(resultsReady()));
    connect(&dns, SIGNAL(resultsReady()), this, SLOT(simpleLookupDone()));
    QTestEventLoop::instance().enterLoop(5);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Network operation timed out");
    QCOMPARE(spy.count(), 1);
}

void tst_Q3Dns::simpleLookupDone()
{
    QTestEventLoop::instance().exitLoop();
}


QTEST_APPLESS_MAIN(tst_Q3Dns)
#include "tst_q3dns.moc"

