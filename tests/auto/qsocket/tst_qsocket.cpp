/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>


#include <qapplication.h>




#include <q3socket.h>

//TESTED_CLASS=
//TESTED_FILES=network/q3socket.h network/q3socket.cpp

class tst_Q3Socket : public QObject
{
    Q_OBJECT

public:
    tst_Q3Socket();
    virtual ~tst_Q3Socket();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void peerAddress_data();
    void peerAddress();
    void emitConnectionRefused();
    void readBufferSize();
    void connectionAttempts_data();
    void connectionAttempts();
    void canReadLine();

protected slots:
    void peerAddress_connected();
    void peerAddress_connectionClosed();
    void emitConnectionRefused_error(int);
    void connectionAttempts_connected();
    void connectionAttempts_error(int);

private:
    Q3Socket *socket;

    QHostAddress *peerAddress_addrConnected;
    uint peerAddress_portConnected;
    QHostAddress *peerAddress_addrClosed;
    uint peerAddress_portClosed;
    bool emitConnectionRefused_errorReceived;
};

tst_Q3Socket::tst_Q3Socket()
{
}

tst_Q3Socket::~tst_Q3Socket()
{
}

void tst_Q3Socket::initTestCase()
{
}

void tst_Q3Socket::cleanupTestCase()
{
}

void tst_Q3Socket::init()
{
}

void tst_Q3Socket::cleanup()
{
}

void tst_Q3Socket::peerAddress_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<uint>("port");
    QTest::addColumn<bool>("peerClosesConnection");
    QTest::addColumn<QString>("peerAddr");

    QTest::newRow( "echo" )    << QString("fluke.troll.no") << (uint)7  << false << QString("10.3.3.31");
    QTest::newRow( "daytime" ) << QString("fluke.troll.no") << (uint)13 << true << QString("10.3.3.31");
}

void tst_Q3Socket::peerAddress()
{
    // init
    QFETCH( QString, host );
    QFETCH( uint, port );
    QFETCH( QString, peerAddr );
    QFETCH( bool, peerClosesConnection );

    socket = new Q3Socket;
    connect( socket, SIGNAL(connected()), SLOT(peerAddress_connected()) );
    connect( socket, SIGNAL(connectionClosed()), SLOT(peerAddress_connectionClosed()) );

    peerAddress_addrConnected = 0;
    peerAddress_addrClosed = 0;

    // connect to host
    socket->connectToHost( host, port );
    QTestEventLoop::instance().enterLoop( 30 );
    if ( QTestEventLoop::instance().timeout() && peerAddress_addrConnected==0 )
	QFAIL( "Connection timed out" );

    // test
    QHostAddress pa;
    QVERIFY( pa.setAddress(peerAddr) );

    QTEST( peerAddress_addrConnected->toString(), "peerAddr" ); // results in nicer output than the test below
    QVERIFY( *peerAddress_addrConnected == pa );
    QCOMPARE( peerAddress_portConnected, port );

    if ( peerClosesConnection ) {
	QVERIFY( peerAddress_addrClosed != 0 );

	QCOMPARE( peerAddress_addrClosed->toString(), QString() ); // results in nicer output than the test below
	QVERIFY( *peerAddress_addrClosed == QHostAddress::Null );
	QCOMPARE( peerAddress_portClosed, (uint)0 );
    } else {
	QVERIFY( peerAddress_addrClosed == 0 );
    }

    // cleanup
    delete peerAddress_addrConnected;
    delete peerAddress_addrClosed;
    delete socket;
}

void tst_Q3Socket::peerAddress_connected()
{
    peerAddress_addrConnected = new QHostAddress( socket->peerAddress() );
    peerAddress_portConnected = socket->peerPort();
    QTestEventLoop::instance().changeInterval( 5 ); // enough time to get the closed
}

void tst_Q3Socket::peerAddress_connectionClosed()
{
    peerAddress_addrClosed = new QHostAddress( socket->peerAddress() );
    peerAddress_portClosed = socket->peerPort();
    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Socket::emitConnectionRefused()
{
    Q3Socket sock;
    connect( &sock, SIGNAL(error(int)), SLOT(emitConnectionRefused_error(int)) );
    sock.connectToHost( "ares.troll.no", 12331 );

    emitConnectionRefused_errorReceived = false;
    QTestEventLoop::instance().enterLoop( 30 );
    QVERIFY(emitConnectionRefused_errorReceived);
}

void tst_Q3Socket::emitConnectionRefused_error( int signum )
{
    if ( signum == Q3Socket::ErrConnectionRefused )
        emitConnectionRefused_errorReceived = true;

    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Socket::readBufferSize()
{
    const int bufferSize = 1024;

    Q3Socket sock;
    sock.setReadBufferSize(bufferSize);
    QCOMPARE((int)sock.readBufferSize(), bufferSize);

    sock.connectToHost("localhost", 0);

    QCOMPARE((int)sock.readBufferSize(), bufferSize);
}

void tst_Q3Socket::connectionAttempts_data()
{
    QTest::addColumn<QString>("host");
    QTest::addColumn<int>("port");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("fluke port 80") << QString("fluke.troll.no") << 80 << true;
    QTest::newRow("fluke port 79") << QString("fluke.troll.no") << 79 << false;
}

void tst_Q3Socket::connectionAttempts()
{
    QFETCH(QString, host);
    QFETCH(int, port);
    QFETCH(bool, expectedResult);

    Q3Socket sock;
    sock.connectToHost(host, port);
    connect(&sock, SIGNAL(connected()), SLOT(connectionAttempts_connected()));
    connect(&sock, SIGNAL(error(int)), SLOT(connectionAttempts_error(int)));
    QTestEventLoop::instance().enterLoop(10);

    if (QTestEventLoop::instance().timeout())
       return;

    QCOMPARE(sock.state() == Q3Socket::Connected, expectedResult);
}

void tst_Q3Socket::connectionAttempts_connected()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Socket::connectionAttempts_error(int)
{
    QTestEventLoop::instance().exitLoop();
}

void tst_Q3Socket::canReadLine()
{
    QEventLoop loop;

    Q3Socket socket;
    QVERIFY(!socket.canReadLine());
    connect(&socket, SIGNAL(connected()), &loop, SLOT(quit()));
    socket.connectToHost("fluke.troll.no", 143);

    loop.exec();

    QCOMPARE(socket.state(), Q3Socket::Connected);

    while (!socket.canReadLine())
        QVERIFY(socket.waitForMore(5000) > 0);

    QVERIFY(socket.canReadLine());
    socket.readLine();
    QVERIFY(!socket.canReadLine());
    socket.ungetChar('\n');
    QVERIFY(socket.canReadLine());
}

QTEST_MAIN(tst_Q3Socket)
#include "tst_qsocket.moc"
