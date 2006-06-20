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




#include <q3serversocket.h>
#include <q3socket.h>

//TESTED_CLASS=
//TESTED_FILES=network/q3serversocket.h network/q3serversocket.cpp

class TestServer : public Q3ServerSocket
{
    Q_OBJECT
public:

    TestServer( int port );
    ~TestServer();

    void newConnection( int socket );

signals:
    void acceptedClient( int socket );
};

class tst_Q3ServerSocket : public QObject
{
    Q_OBJECT

public:
    tst_Q3ServerSocket();
    virtual ~tst_Q3ServerSocket();


public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
private slots:
    void accept();

protected slots:
    void acceptClient( int socket );

private:
    TestServer *serverSocket;
    Q3Socket *socket;
};

tst_Q3ServerSocket::tst_Q3ServerSocket()
{
}

tst_Q3ServerSocket::~tst_Q3ServerSocket()
{
}

void tst_Q3ServerSocket::initTestCase()
{
}

void tst_Q3ServerSocket::cleanupTestCase()
{
}

void tst_Q3ServerSocket::init()
{
}

void tst_Q3ServerSocket::cleanup()
{
}

void tst_Q3ServerSocket::accept()
{
    // init
    serverSocket = new TestServer( 12345 );
    connect( serverSocket, SIGNAL( acceptedClient( int ) ), SLOT( acceptClient( int ) ) );

    QVERIFY( serverSocket->port() == 12345 );

    socket = new Q3Socket;
    socket->connectToHost( "localhost", 12345 );

    QTestEventLoop::instance().enterLoop( 30 );

    delete serverSocket;
    delete socket;
}

void tst_Q3ServerSocket::acceptClient( int )
{
    QTestEventLoop::instance().exitLoop();
}

TestServer::TestServer( int port ) : Q3ServerSocket( port )
{
}

TestServer::~TestServer()
{
}

void TestServer::newConnection( int socket )
{
    emit acceptedClient( socket );
}

QTEST_MAIN(tst_Q3ServerSocket)
#include "tst_qserversocket.moc"
