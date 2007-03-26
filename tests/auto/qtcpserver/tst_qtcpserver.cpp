/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#include <QtTest/QtTest>

#ifndef Q_OS_WIN
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include <qcoreapplication.h>
#include <qtcpsocket.h>
#include <qtcpserver.h>
#include <qhostaddress.h>
#include <qprocess.h>
#include <qstringlist.h>
#include <qplatformdefs.h>
#ifdef TEST_QNETWORK_PROXY
# include <QNetworkProxy>
#endif

//TESTED_FILES=network/qtcpserver.cpp network/qtcpserver.h network/qtcpsocket.cpp network/qtcpsocket.h network/qabstractsocket.cpp network/qabstractsocket.h network/qabstractsocket_p.h

class tst_QTcpServer : public QObject
{
    Q_OBJECT

public:
    tst_QTcpServer();
    virtual ~tst_QTcpServer();


public slots:
    void initTestCase_data();
    void init();
    void cleanup();
private slots:
    void getSetCheck();
    void constructing();
    void clientServerLoop();
    void ipv6Server();
    void ipv4LoopbackPerformanceTest();
    void ipv6LoopbackPerformanceTest();
    void ipv4PerformanceTest();
    void crashTests();
    void maxPendingConnections();
    void listenError();
    void waitForConnectionTest();
    void setSocketDescriptor();
    void listenWhileListening();
    void addressReusable();
    void setNewSocketDescriptorBlocking();
};

// Testing get/set functions
void tst_QTcpServer::getSetCheck()
{
    QTcpServer obj1;
    // int QTcpServer::maxPendingConnections()
    // void QTcpServer::setMaxPendingConnections(int)
    obj1.setMaxPendingConnections(0);
    QCOMPARE(0, obj1.maxPendingConnections());
    obj1.setMaxPendingConnections(INT_MIN);
    QCOMPARE(INT_MIN, obj1.maxPendingConnections());
    obj1.setMaxPendingConnections(INT_MAX);
    QCOMPARE(INT_MAX, obj1.maxPendingConnections());
}

tst_QTcpServer::tst_QTcpServer()
{
}

tst_QTcpServer::~tst_QTcpServer()
{

}

void tst_QTcpServer::initTestCase_data()
{
    QTest::addColumn<bool>("setProxy");
    QTest::addColumn<int>("proxyType");  
    
    QTest::newRow("WithoutProxy") << false << 0;
#ifdef TEST_QNETWORK_PROXY
    QTest::newRow("WithSocks5Proxy") << true << int(QNetworkProxy::Socks5Proxy);
#endif
}

void tst_QTcpServer::init()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, "fluke.troll.no", 1080));
        }
#endif
    }
}

void tst_QTcpServer::cleanup()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QNetworkProxy::setApplicationProxy(QNetworkProxy::DefaultProxy);
#endif
    }
}

//----------------------------------------------------------------------------------

void tst_QTcpServer::constructing()
{
    QTcpServer socket;

    // Check the initial state of the QTcpSocket.
    QCOMPARE(socket.isListening(), false);
    QCOMPARE((int)socket.serverPort(), 0);
    QCOMPARE(socket.serverAddress(), QHostAddress());
    QCOMPARE(socket.maxPendingConnections(), 30);
    QCOMPARE(socket.hasPendingConnections(), false);
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE(socket.serverError(), QAbstractSocket::UnknownSocketError);

    // Check the state of the socket layer?
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::clientServerLoop()
{
    QTcpServer server;

    QSignalSpy spy(&server, SIGNAL(newConnection()));

    QVERIFY(!server.isListening());
    QVERIFY(!server.hasPendingConnections());
    QVERIFY(server.listen(QHostAddress::Any, 11423));
    QVERIFY(server.isListening());

    QTcpSocket client;

    QHostAddress serverAddress = QHostAddress::LocalHost;
    if (!(server.serverAddress() == QHostAddress::Any))
        serverAddress = server.serverAddress();

    client.connectToHost(serverAddress, server.serverPort());
    QVERIFY(client.waitForConnected(5000));

    QVERIFY(server.waitForNewConnection(5000));
    QVERIFY(server.hasPendingConnections());
    
    QCOMPARE(spy.count(), 1);

    QTcpSocket *serverSocket = server.nextPendingConnection();
    QVERIFY(serverSocket != 0);

    QVERIFY(serverSocket->write("Greetings, client!\n", 19) == 19);
    serverSocket->flush();

    QVERIFY(client.waitForReadyRead(5000));
    QByteArray arr = client.readAll();
    QCOMPARE(arr.constData(), "Greetings, client!\n");

    QVERIFY(client.write("Well, hello to you!\n", 20) == 20);
    client.flush();

    QVERIFY(serverSocket->waitForReadyRead(5000));
    arr = serverSocket->readAll();
    QCOMPARE(arr.constData(), "Well, hello to you!\n");
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::ipv6Server()
{
    //### need to enter the event loop for the server to get the connection ?? ( windows)
    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHostIPv6, 8944)) {
        QVERIFY(server.serverError() == QAbstractSocket::UnsupportedSocketOperationError);
        return;
    }

    QVERIFY(server.serverPort() == 8944);
    QVERIFY(server.serverAddress() == QHostAddress::LocalHostIPv6);

    QTcpSocket client;
    client.connectToHost("::1", 8944);
    QVERIFY(client.waitForConnected(5000));

    QVERIFY(server.waitForNewConnection());
    QVERIFY(server.hasPendingConnections());

    QTcpSocket *serverSocket = 0;
    QVERIFY((serverSocket = server.nextPendingConnection()));
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::ipv4LoopbackPerformanceTest()
{
    
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QVERIFY(server.isListening());

    QTcpSocket clientA;
    clientA.connectToHost(QHostAddress::LocalHost, server.serverPort());
    QVERIFY(clientA.waitForConnected(5000));
    QVERIFY(clientA.state() == QAbstractSocket::ConnectedState);
    
    QVERIFY(server.waitForNewConnection());
    QTcpSocket *clientB = server.nextPendingConnection();
    QVERIFY(clientB);

#if defined(Q_WS_WIN)
    if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
        QSKIP("Dont run performance tests on QSysInfo::WV_DOS_based systems, overloads the system", SkipAll);
    } else
#endif
    {
        QByteArray buffer(16384, '@');
        QTime stopWatch;
        stopWatch.start();
        qlonglong totalWritten = 0;
        while (stopWatch.elapsed() < 5000) {
            QVERIFY(clientA.write(buffer.data(), buffer.size()) > 0);
            clientA.flush();
            totalWritten += buffer.size();
            while (clientB->waitForReadyRead(100)) {
                if (clientB->bytesAvailable() == 16384)
                    break;
            }
            clientB->read(buffer.data(), buffer.size());
            clientB->write(buffer.data(), buffer.size());
            clientB->flush();
            totalWritten += buffer.size();
            while (clientA.waitForReadyRead(100)) {
                if (clientA.bytesAvailable() == 16384)
                    break;
            }
            clientA.read(buffer.data(), buffer.size());
        }

        qDebug("\t\t%s: %.1fMB/%.1fs: %.1fMB/s",
               server.serverAddress().toString().toLatin1().constData(),
               totalWritten / (1024.0 * 1024.0),
               stopWatch.elapsed() / 1000.0,
               (totalWritten / (stopWatch.elapsed() / 1000.0)) / (1024 * 1024));

    }
    delete clientB;
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::ipv6LoopbackPerformanceTest()
{
    QTcpServer server;
    if (!server.listen(QHostAddress::LocalHostIPv6, 0)) {
        QVERIFY(server.serverError() == QAbstractSocket::UnsupportedSocketOperationError);
    } else {
        QTcpSocket clientA;
        clientA.connectToHost(server.serverAddress(), server.serverPort());
        QVERIFY(clientA.waitForConnected(5000));

        QVERIFY(server.waitForNewConnection(5000));
        QTcpSocket *clientB = server.nextPendingConnection();
        QVERIFY(clientB);

        QByteArray buffer(16384, '@');
        QTime stopWatch;
        stopWatch.start();
        qlonglong totalWritten = 0;
        while (stopWatch.elapsed() < 5000) {
            clientA.write(buffer.data(), buffer.size());
            clientA.flush();
            totalWritten += buffer.size();
            while (clientB->waitForReadyRead(100)) {
                if (clientB->bytesAvailable() == 16384)
                    break;
            }
            clientB->read(buffer.data(), buffer.size());
            clientB->write(buffer.data(), buffer.size());
            clientB->flush();
            totalWritten += buffer.size();
            while (clientA.waitForReadyRead(100)) {
                if (clientA.bytesAvailable() == 16384)
                   break;
            }
            clientA.read(buffer.data(), buffer.size());
        }

        qDebug("\t\t%s: %.1fMB/%.1fs: %.1fMB/s",
               server.serverAddress().toString().toLatin1().constData(),
               totalWritten / (1024.0 * 1024.0),
               stopWatch.elapsed() / 1000.0,
               (totalWritten / (stopWatch.elapsed() / 1000.0)) / (1024 * 1024));
        delete clientB;
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::ipv4PerformanceTest()
{
    QTcpSocket probeSocket;
    probeSocket.connectToHost("imap", 143);
    QVERIFY(probeSocket.waitForConnected(5000));

    QTcpServer server;
    QVERIFY(server.listen(probeSocket.localAddress(), 0));

    QTcpSocket clientA;
    clientA.connectToHost(server.serverAddress(), server.serverPort());
    QVERIFY(clientA.waitForConnected(5000));

    QVERIFY(server.waitForNewConnection(5000));
    QTcpSocket *clientB = server.nextPendingConnection();
    QVERIFY(clientB);

#if defined(Q_WS_WIN)
    if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based) {
        QSKIP("Dont run performance tests on QSysInfo::WV_DOS_based systems, overloads the system", SkipAll);
    } else
#endif
    {

        QByteArray buffer(16384, '@');
        QTime stopWatch;
        stopWatch.start();
        qlonglong totalWritten = 0;
        while (stopWatch.elapsed() < 5000) {
            qlonglong writtenA = clientA.write(buffer.data(), buffer.size());
            clientA.flush();
            totalWritten += buffer.size();
            while (clientB->waitForReadyRead(100)) {
                if (clientB->bytesAvailable() == writtenA)
                    break;
            }
            clientB->read(buffer.data(), buffer.size());
            qlonglong writtenB = clientB->write(buffer.data(), buffer.size());
            clientB->flush();
            totalWritten += buffer.size();
            while (clientA.waitForReadyRead(100)) {
                if (clientA.bytesAvailable() == writtenB)
                   break;
            }
            clientA.read(buffer.data(), buffer.size());
        }

        qDebug("\t\t%s: %.1fMB/%.1fs: %.1fMB/s",
               probeSocket.localAddress().toString().toLatin1().constData(),
               totalWritten / (1024.0 * 1024.0),
               stopWatch.elapsed() / 1000.0,
               (totalWritten / (stopWatch.elapsed() / 1000.0)) / (1024 * 1024));
    }
    delete clientB;
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::crashTests()
{
    QTcpServer server;
    server.close();
    QVERIFY(server.listen());
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::maxPendingConnections()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QSKIP("With socks5 only 1 connection is allowed ever", SkipAll);
        }
#endif
    }
    //### sees to fail sometimes ... a timing issue with the test on windows
    QTcpServer server;
    server.setMaxPendingConnections(2);

    QTcpSocket socket1;
    QTcpSocket socket2;
    QTcpSocket socket3;

    QVERIFY(server.listen());

    socket1.connectToHost(QHostAddress::LocalHost, server.serverPort());
    socket2.connectToHost(QHostAddress::LocalHost, server.serverPort());
    socket3.connectToHost(QHostAddress::LocalHost, server.serverPort());

    QVERIFY(server.waitForNewConnection(5000));

    QVERIFY(server.hasPendingConnections());
    QVERIFY(server.nextPendingConnection());
    QVERIFY(server.hasPendingConnections());
    QVERIFY(server.nextPendingConnection());
    QVERIFY(!server.hasPendingConnections());
    QCOMPARE(server.nextPendingConnection(), (QTcpSocket*)0);

    QVERIFY(server.waitForNewConnection(5000));

    QVERIFY(server.hasPendingConnections());
    QVERIFY(server.nextPendingConnection());
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::listenError()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QSKIP("With socks5 we can not make hard requirements on the address or port", SkipAll);
        }
#endif
    }
    QTcpServer server;
    QVERIFY(!server.listen(QHostAddress("1.2.3.4"), 0));
    QCOMPARE(server.serverError(), QAbstractSocket::SocketAddressNotAvailableError);
    QCOMPARE(server.errorString().toLatin1().constData(), "The address is not available");
}

class ThreadConnector : public QThread
{
public:
    ThreadConnector(const QHostAddress &host, quint16 port)
        : host(host), port(port)
    { }

    ~ThreadConnector()
    {
        wait();
    }

protected:
    void run()
    {
        sleep(2);

        QTcpSocket socket;
        socket.connectToHost(host, port);

        QEventLoop loop;
        QTimer::singleShot(5000, &loop, SLOT(quit()));
        loop.exec();
    }

private:
    QHostAddress host;
    quint16 port;
};

//----------------------------------------------------------------------------------
void tst_QTcpServer::waitForConnectionTest()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QSKIP("Localhost servers don't work well with SOCKS5", SkipAll);
        }
#endif
    }

    QTcpSocket findLocalIpSocket;
    findLocalIpSocket.connectToHost("fluke.troll.no", 21);
    QVERIFY(findLocalIpSocket.waitForConnected(2000));

    QTcpServer server;
    bool timeout = false;
    QVERIFY(server.listen(findLocalIpSocket.localAddress()));
    QVERIFY(!server.waitForNewConnection(1000, &timeout));
    QCOMPARE(server.serverError(), QAbstractSocket::SocketTimeoutError);
    QVERIFY(timeout);

    ThreadConnector connector(findLocalIpSocket.localAddress(), server.serverPort());
    connector.start();

    QVERIFY(server.waitForNewConnection(3000, &timeout));
    QVERIFY(!timeout);
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::setSocketDescriptor()
{
    QTcpServer server;
    QVERIFY(!server.setSocketDescriptor(42));
    QCOMPARE(server.serverError(), QAbstractSocket::UnsupportedSocketOperationError);

#ifdef Q_OS_WIN
    // ensure winsock is started
    WSADATA wsaData;
    QVERIFY(WSAStartup(MAKEWORD(2,0), &wsaData) == NO_ERROR);
#endif

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    QVERIFY(sock != INVALID_SOCKET);

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    sin.sin_addr.s_addr = 0x00000000;
    QVERIFY(::bind(sock, (sockaddr*)&sin, sizeof(sockaddr_in)) == 0);
    QVERIFY(::listen(sock, 10) == 0);
    QVERIFY(server.setSocketDescriptor(sock));

#ifdef Q_OS_WIN
    WSACleanup();
#endif
}

//----------------------------------------------------------------------------------
void tst_QTcpServer::listenWhileListening()
{
    QTcpServer server;
    QVERIFY(server.listen());
    QTest::ignoreMessage(QtWarningMsg, "QTcpServer::listen() called when already listening");
    QVERIFY(!server.listen());
}

//----------------------------------------------------------------------------------

class SeverWithBlockingSockets : public QTcpServer
{
public:
    SeverWithBlockingSockets()
        : ok(false) { }

    bool ok;

protected:
    void incomingConnection(int socketDescriptor)
    {
        // how a user woulddo it (qabstractsocketengine is not public)
        unsigned long arg = 0;
#ifdef Q_OS_WIN
        ok = ::ioctlsocket(socketDescriptor, FIONBIO, &arg) == 0;
        ::closesocket(socketDescriptor);
#else
        ok = ::ioctl(socketDescriptor, FIONBIO, &arg) == 0;
        ::close(socketDescriptor);
#endif
    }
};

void tst_QTcpServer::addressReusable()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QSKIP("With socks5 this test does not make senans at the momment", SkipAll);
        }
#endif
    }
    // The crashingServer process will crash once it gets a connection.
    QProcess process;
    process.start("crashingServer/crashingServer");
    QVERIFY(process.waitForReadyRead(5000));

    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 49199);
    QVERIFY(socket.waitForConnected(5000));

    QVERIFY(process.waitForFinished(5000));
    
    // Give the system some time.
    QTest::qSleep(10);
    
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost, 49199));
}

void tst_QTcpServer::setNewSocketDescriptorBlocking()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QSKIP("With socks5 we can not make the socket descripter blocking", SkipAll);
        }
#endif
    }
    SeverWithBlockingSockets server;
    QVERIFY(server.listen());

    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, server.serverPort());

    QVERIFY(server.waitForNewConnection(5000));

    QVERIFY(server.ok);
}

QTEST_MAIN(tst_QTcpServer)
#include "tst_qtcpserver.moc"
