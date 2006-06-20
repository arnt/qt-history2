/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qcoreapplication.h>

#include <private/qsocks5socketengine_p.h>
#include <qhostinfo.h>
#include <qhostaddress.h>
#include <qtcpsocket.h>
#include <qhttp.h>
#include <qdebug.h>

class tst_QSocks5SocketEngine : public QObject
{
    Q_OBJECT

public:
    tst_QSocks5SocketEngine();
    virtual ~tst_QSocks5SocketEngine();


public slots:
    void init();
    void cleanup();
private slots:
    void construction();
    void simpleConnectToIMAP();
    void simpleErrorsAndStates();
    void udpTest();
    void serverTest();
    void tcpSocketBlockingTest();
    void tcpSocketNonBlockingTest();
    void downloadBigFile();
   // void tcpLoopbackPerformance();
    void passwordAuth();
    
protected slots:
    void tcpSocketNonBlocking_hostFound();
    void tcpSocketNonBlocking_connected();
    void tcpSocketNonBlocking_closed();
    void tcpSocketNonBlocking_readyRead();
    void tcpSocketNonBlocking_bytesWritten(qint64);
    void exitLoopSlot();
    void downloadBigFileSlot();

private:
    QTcpSocket *tcpSocketNonBlocking_socket;
    QStringList tcpSocketNonBlocking_data;
    qint64 tcpSocketNonBlocking_totalWritten;
    QTcpSocket *tmpSocket;
    qint64 bytesAvailable;
};

static const char *IMAP_IP = "62.70.27.18";
static const char *PROXY_IP = "smokesignal.troll.no";

tst_QSocks5SocketEngine::tst_QSocks5SocketEngine()
{
}

tst_QSocks5SocketEngine::~tst_QSocks5SocketEngine()
{

}

void tst_QSocks5SocketEngine::init()
{
    tmpSocket = 0;
    bytesAvailable = 0;
}

void tst_QSocks5SocketEngine::cleanup()
{
}

//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::construction()
{
    QSocks5SocketEngine socketDevice;

    QVERIFY(!socketDevice.isValid());

    // Initialize device
    QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(socketDevice.isValid());
    QVERIFY(socketDevice.protocol() == QAbstractSocket::IPv4Protocol);
    QVERIFY(socketDevice.socketType() == QAbstractSocket::TcpSocket);
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
   // QVERIFY(socketDevice.socketDescriptor() != -1);
    QVERIFY(socketDevice.localAddress() == QHostAddress());
    QVERIFY(socketDevice.localPort() == 0);
    QVERIFY(socketDevice.peerAddress() == QHostAddress());
    QVERIFY(socketDevice.peerPort() == 0);
    QVERIFY(socketDevice.error() == QAbstractSocket::UnknownSocketError);

    //QTest::ignoreMessage(QtWarningMsg, "QSocketLayer::bytesAvailable() was called in QAbstractSocket::UnconnectedState");
    QVERIFY(socketDevice.bytesAvailable() == 0);

    //QTest::ignoreMessage(QtWarningMsg, "QSocketLayer::hasPendingDatagrams() was called in QAbstractSocket::UnconnectedState");
    QVERIFY(!socketDevice.hasPendingDatagrams());
}
    
//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::simpleConnectToIMAP()
{
    QSocks5SocketEngine socketDevice;

    // Initialize device
    QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);

    socketDevice.setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, PROXY_IP, 1080));

    // Connect to imap.trolltech.com's IP
    QVERIFY(!socketDevice.connectToHost(QHostAddress(IMAP_IP), 143));
    QVERIFY(socketDevice.state() == QAbstractSocket::ConnectingState);
    QVERIFY(socketDevice.waitForWrite());
    QVERIFY(socketDevice.connectToHost(QHostAddress(IMAP_IP), 143));
    QVERIFY(socketDevice.state() == QAbstractSocket::ConnectedState);
    QVERIFY(socketDevice.peerAddress() == QHostAddress(IMAP_IP));

    // Wait for the greeting
    QVERIFY(socketDevice.waitForRead());

    // Read the greeting
    qint64 available = socketDevice.bytesAvailable();
    QVERIFY(available > 0);
    QByteArray array;
    array.resize(available);
    QVERIFY(socketDevice.read(array.data(), array.size()) == available);

    // Check that the greeting is what we expect it to be
    QCOMPARE(array.constData(),
            "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");

    // Write a logout message
    QByteArray array2 = "XXXX LOGOUT\r\n";
    QVERIFY(socketDevice.write(array2.data(),
                              array2.size()) == array2.size());

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());

    available = socketDevice.bytesAvailable();
    QVERIFY(available > 0);
    array.resize(available);
    QVERIFY(socketDevice.read(array.data(), array.size()) == available);

    // Check that the greeting is what we expect it to be
    QCOMPARE(array.constData(), "* BYE LOGOUT received\r\nXXXX OK Completed\r\n");

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());
    char c;
    QVERIFY(socketDevice.read(&c, sizeof(c)) == -1);
    QVERIFY(socketDevice.error() == QAbstractSocket::RemoteHostClosedError);
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
}

//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::simpleErrorsAndStates()
{
    {
        QSocks5SocketEngine socketDevice;

        // Initialize device
        QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
        
        socketDevice.setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, PROXY_IP, 1080));

        QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
        QVERIFY(!socketDevice.connectToHost(QHostInfo::fromName(PROXY_IP).addresses().first(), 8088));
        QVERIFY(socketDevice.state() == QAbstractSocket::ConnectingState);
        if (socketDevice.waitForWrite(15000)) {
            if (!socketDevice.connectToHost(QHostInfo::fromName(PROXY_IP).addresses().first(), 8088)) {
                QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
                //QVERIFY(socketDevice.error() == QAbstractSocket::SocketTimeoutError);
            } else {
                QVERIFY(socketDevice.state() == QAbstractSocket::ConnectedState);
            }
        } else {
            QVERIFY(socketDevice.error() == QAbstractSocket::SocketTimeoutError);
        }
    }

}

/*
//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::tcpLoopbackPerformance()
{
    QTcpServer server;

    // Bind to any port on all interfaces
    QVERIFY(server.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(server.state() == QAbstractSocket::BoundState);
    quint16 port = server.localPort();

    // Listen for incoming connections
    QVERIFY(server.listen());
    QVERIFY(server.state() == QAbstractSocket::ListeningState);

    // Initialize a Tcp socket
    QSocks5SocketEngine client;
    QVERIFY(client.initialize(QAbstractSocket::TcpSocket));

    client.setProxy(QHostAddress("80.232.37.158"), 1081);

    // Connect to our server
    if (!client.connectToHost(QHostAddress("127.0.0.1"), port)) {
        QVERIFY(client.waitForWrite());
        QVERIFY(client.connectToHost(QHostAddress("127.0.0.1"), port));
    }

    // The server accepts the connectio
    int socketDescriptor = server.accept();
    QVERIFY(socketDescriptor > 0);

    // A socket device is initialized on the server side, passing the
    // socket descriptor from accept(). It's pre-connected.
    QSocketLayer serverSocket;
    QVERIFY(serverSocket.initialize(socketDescriptor));
    QVERIFY(serverSocket.state() == QAbstractSocket::ConnectedState);

    const int messageSize = 1024 * 256;
    QByteArray message1(messageSize, '@');
    QByteArray answer(messageSize, '@');

    QTime timer;
    timer.start();
    qlonglong readBytes = 0;
    while (timer.elapsed() < 5000) {
        qlonglong written = serverSocket.write(message1.data(), message1.size());
        while (written > 0) {
            client.waitForRead();
            if (client.bytesAvailable() > 0) {
                qlonglong readNow = client.read(answer.data(), answer.size());
                written -= readNow;
                readBytes += readNow;
            }
        }
    }

    qDebug("\t\t%.1fMB/%.1fs: %.1fMB/s",
           readBytes / (1024.0 * 1024.0),
           timer.elapsed() / 1024.0,
           (readBytes / (timer.elapsed() / 1000.0)) / (1024 * 1024));
}
*/

//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::serverTest()
{
    QSocks5SocketEngine server;

    // Initialize a Tcp socket
    QVERIFY(server.initialize(QAbstractSocket::TcpSocket));

    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, PROXY_IP, 1080);

    server.setProxy(proxy);

    // Bind to any port on all interfaces
    QVERIFY(server.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(server.state() == QAbstractSocket::BoundState);
    
    // Listen for incoming connections
    QVERIFY(server.listen());
    QVERIFY(server.state() == QAbstractSocket::ListeningState);

    // Initialize a Tcp socket
    QSocks5SocketEngine client;
    QVERIFY(client.initialize(QAbstractSocket::TcpSocket));
    
    client.setProxy(proxy);

   // QTest::wait(100000); // ### timing problem on win32
        

    // Connect to our server
    if (!client.connectToHost(server.localAddress(), server.localPort())) {
        QVERIFY(client.waitForWrite());
       // QTest::wait(100); // ### timing problem on win32
        QVERIFY(client.connectToHost(server.localAddress(), server.localPort()));
        //QTest::wait(100); 
    }

    QVERIFY(server.waitForRead());

    // The server accepts the connection
    int socketDescriptor = server.accept();
    QVERIFY(socketDescriptor > 0);

    // A socket device is initialized on the server side, passing the
    // socket descriptor from accept(). It's pre-connected.
    
    QSocks5SocketEngine serverSocket;
    QVERIFY(serverSocket.initialize(socketDescriptor));
    QVERIFY(serverSocket.state() == QAbstractSocket::ConnectedState);

    QVERIFY(serverSocket.localAddress() == client.peerAddress());
    QVERIFY(serverSocket.localPort() == client.peerPort());
    // this seems depends on the socks server implementation, especially
    // when connecting /to/ the socks server /through/ the same socks server
    //QVERIFY(serverSocket.peerAddress() == client.localAddress());
    //QVERIFY(serverSocket.peerPort() == client.localPort());

    // The server socket sends a greeting to the client
    QByteArray greeting = "Greetings!";
    QVERIFY(serverSocket.write(greeting.data(),
                              greeting.size()) == greeting.size());

    // The client waits for the greeting to arrive
    QVERIFY(client.waitForRead());
    qint64 available = client.bytesAvailable();
    QVERIFY(available > 0);

    // The client reads the greeting and checks that it's correct
    QByteArray response;
    response.resize(available);
    QVERIFY(client.read(response.data(),
                       response.size()) == response.size());
    QCOMPARE(response, greeting);
}


//---------------------------------------------------------------------------
void tst_QSocks5SocketEngine::udpTest()
{
    QSocks5SocketEngine udpSocket;

    // Initialize device #1
    QVERIFY(udpSocket.initialize(QAbstractSocket::UdpSocket));
    QVERIFY(udpSocket.isValid());

    QNetworkProxy proxy(QNetworkProxy::Socks5Proxy, PROXY_IP, 1080);

    udpSocket.setProxy(proxy);
    
    QVERIFY(udpSocket.protocol() == QAbstractSocket::IPv4Protocol);
    QVERIFY(udpSocket.socketType() == QAbstractSocket::UdpSocket);
    QVERIFY(udpSocket.state() == QAbstractSocket::UnconnectedState);

    // Bind #1
    QVERIFY(udpSocket.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(udpSocket.state() == QAbstractSocket::BoundState);
    QVERIFY(udpSocket.localPort() != 0);

    // Initialize device #2
    QSocks5SocketEngine udpSocket2;
    QVERIFY(udpSocket2.initialize(QAbstractSocket::UdpSocket));

    udpSocket2.setProxy(proxy);

    // Connect device #2 to #1
    QVERIFY(udpSocket2.connectToHost(udpSocket.localAddress(), udpSocket.localPort()));
    QVERIFY(udpSocket2.state() == QAbstractSocket::ConnectedState);

    // Write a message to #1
    QByteArray message1 = "hei der";
    QVERIFY(udpSocket2.write(message1.data(),
                            message1.size()) == message1.size());

    // Read the message from #2
    QVERIFY(udpSocket.waitForRead());
    QVERIFY(udpSocket.hasPendingDatagrams());
    qint64 available = udpSocket.pendingDatagramSize();
    QVERIFY(available > 0);
    QByteArray answer;
    answer.resize(available);
    QHostAddress senderAddress;
    quint16 senderPort = 0;
    QVERIFY(udpSocket.readDatagram(answer.data(), answer.size(),
                                  &senderAddress,
                                  &senderPort) == message1.size());
    QVERIFY(senderAddress == udpSocket2.localAddress());
    QVERIFY(senderPort == udpSocket2.localPort());
}

void tst_QSocks5SocketEngine::tcpSocketBlockingTest()
{
    QSocks5SocketEngineHandler socks5;

    QTcpSocket socket;

    // Connect
    socket.connectToHost("imap", 143);
    QVERIFY(socket.waitForConnected());
    QCOMPARE(socket.state(), QTcpSocket::ConnectedState);

    // Read greeting
    QVERIFY(socket.waitForReadyRead(5000));
    QString s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");

    // Write NOOP
    QCOMPARE((int) socket.write("1 NOOP\r\n", 8), 8);

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    // Read response
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "1 OK Completed\r\n");

    // Write LOGOUT
    QCOMPARE((int) socket.write("2 LOGOUT\r\n", 10), 10);

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    // Read two lines of respose
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "* BYE LOGOUT received\r\n");

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "2 OK Completed\r\n");

    // Close the socket
    socket.close();

    // Check that it's closed
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

void tst_QSocks5SocketEngine::tcpSocketNonBlockingTest()
{
    QSocks5SocketEngineHandler socks5;
    
    QTcpSocket socket;
    connect(&socket, SIGNAL(hostFound()), SLOT(tcpSocketNonBlocking_hostFound()));
    connect(&socket, SIGNAL(connected()), SLOT(tcpSocketNonBlocking_connected()));
    connect(&socket, SIGNAL(disconnected()), SLOT(tcpSocketNonBlocking_closed()));
    connect(&socket, SIGNAL(bytesWritten(qint64)), SLOT(tcpSocketNonBlocking_bytesWritten(qint64)));
    connect(&socket, SIGNAL(readyRead()), SLOT(tcpSocketNonBlocking_readyRead()));
    tcpSocketNonBlocking_socket = &socket;

    // Connect
    socket.connectToHost("imap", 143);
    QCOMPARE(socket.state(), QTcpSocket::HostLookupState);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    if (socket.state() == QTcpSocket::ConnectingState) {
        QTestEventLoop::instance().enterLoop(30);
        if (QTestEventLoop::instance().timeout()) {
            QFAIL("Timed out");
        }
    }

    QCOMPARE(socket.state(), QTcpSocket::ConnectedState);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    // Read greeting
    QVERIFY(!tcpSocketNonBlocking_data.isEmpty());
    QCOMPARE(tcpSocketNonBlocking_data.at(0).toLatin1().constData(),
            "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");
    tcpSocketNonBlocking_data.clear();

    tcpSocketNonBlocking_totalWritten = 0;

    // Write NOOP
    QCOMPARE((int) socket.write("1 NOOP\r\n", 8), 8);


    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(tcpSocketNonBlocking_totalWritten == 8);


    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }


    // Read response
    QVERIFY(!tcpSocketNonBlocking_data.isEmpty());
    QCOMPARE(tcpSocketNonBlocking_data.at(0).toLatin1().constData(), "1 OK Completed\r\n");
    tcpSocketNonBlocking_data.clear();


    tcpSocketNonBlocking_totalWritten = 0;

    // Write LOGOUT
    QCOMPARE((int) socket.write("2 LOGOUT\r\n", 10), 10);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(tcpSocketNonBlocking_totalWritten == 10);

    // Wait for greeting
    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    // Read two lines of respose
    QCOMPARE(tcpSocketNonBlocking_data.at(0).toLatin1().constData(), "* BYE LOGOUT received\r\n");
    QCOMPARE(tcpSocketNonBlocking_data.at(1).toLatin1().constData(), "2 OK Completed\r\n");
    tcpSocketNonBlocking_data.clear();

    // Close the socket
    socket.close();

    // Check that it's closed
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

void tst_QSocks5SocketEngine::tcpSocketNonBlocking_hostFound()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_QSocks5SocketEngine::tcpSocketNonBlocking_connected()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_QSocks5SocketEngine::tcpSocketNonBlocking_readyRead()
{
    while (tcpSocketNonBlocking_socket->canReadLine())
        tcpSocketNonBlocking_data.append(tcpSocketNonBlocking_socket->readLine());

    QTestEventLoop::instance().exitLoop();
}

void tst_QSocks5SocketEngine::tcpSocketNonBlocking_bytesWritten(qint64 written)
{
    tcpSocketNonBlocking_totalWritten += written;
    QTestEventLoop::instance().exitLoop();
}

void tst_QSocks5SocketEngine::tcpSocketNonBlocking_closed()
{
}

//----------------------------------------------------------------------------------

void tst_QSocks5SocketEngine::downloadBigFile()
{
    QSocks5SocketEngineHandler socks5;

    if (tmpSocket)
        delete tmpSocket;
    tmpSocket = new QTcpSocket;

    connect(tmpSocket, SIGNAL(connected()), SLOT(exitLoopSlot()));
    connect(tmpSocket, SIGNAL(readyRead()), SLOT(downloadBigFileSlot()));

    tmpSocket->connectToHost("ares.troll.no", 80);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Network operation timed out");

    QVERIFY(tmpSocket->state() == QAbstractSocket::ConnectedState);
    QVERIFY(tmpSocket->write("GET /~ahanssen/QTest/mediumfile HTTP/1.0\r\n") > 0);
    QVERIFY(tmpSocket->write("HOST: ares.troll.no\r\n") > 0);
    QVERIFY(tmpSocket->write("\r\n") > 0);

    bytesAvailable = 0;

    QTime stopWatch;
    stopWatch.start();

    QTestEventLoop::instance().enterLoop(60);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Network operation timed out");

    QCOMPARE(bytesAvailable, qint64(10000271));

    QVERIFY(tmpSocket->state() == QAbstractSocket::ConnectedState);

    qDebug("\t\t%.1fMB/%.1fs: %.1fMB/s",
           bytesAvailable / (1024.0 * 1024.0),
           stopWatch.elapsed() / 1024.0,
           (bytesAvailable / (stopWatch.elapsed() / 1000.0)) / (1024 * 1024));

    delete tmpSocket;
    tmpSocket = 0;
}

void tst_QSocks5SocketEngine::exitLoopSlot()
{
    QTestEventLoop::instance().exitLoop();
}


void tst_QSocks5SocketEngine::downloadBigFileSlot()
{
    bytesAvailable += tmpSocket->readAll().size();
    if (bytesAvailable == 10000271)
        QTestEventLoop::instance().exitLoop();
}

void tst_QSocks5SocketEngine::passwordAuth()
{
    QSocks5SocketEngine socketDevice;

    // Initialize device
    QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);

    socketDevice.setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, PROXY_IP, 1080, "qsockstest", "qsockstest"));

    // Connect to imap.trolltech.com's IP
    QVERIFY(!socketDevice.connectToHost(QHostAddress(IMAP_IP), 143));
    QVERIFY(socketDevice.state() == QAbstractSocket::ConnectingState);
    QVERIFY(socketDevice.waitForWrite());
    if (!socketDevice.connectToHost(QHostAddress(IMAP_IP), 143)) {
        qDebug("%d, %s", socketDevice.error(), socketDevice.errorString().toLatin1().constData());
    }
    QVERIFY(socketDevice.state() == QAbstractSocket::ConnectedState);
    QVERIFY(socketDevice.peerAddress() == QHostAddress(IMAP_IP));

    // Wait for the greeting
    QVERIFY(socketDevice.waitForRead());

    // Read the greeting
    qint64 available = socketDevice.bytesAvailable();
    QVERIFY(available > 0);
    QByteArray array;
    array.resize(available);
    QVERIFY(socketDevice.read(array.data(), array.size()) == available);

    // Check that the greeting is what we expect it to be
    QCOMPARE(array.constData(),
            "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");

    // Write a logout message
    QByteArray array2 = "XXXX LOGOUT\r\n";
    QVERIFY(socketDevice.write(array2.data(),
                              array2.size()) == array2.size());

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());

    available = socketDevice.bytesAvailable();
    QVERIFY(available > 0);
    array.resize(available);
    QVERIFY(socketDevice.read(array.data(), array.size()) == available);

    // Check that the greeting is what we expect it to be
    QCOMPARE(array.constData(), "* BYE LOGOUT received\r\nXXXX OK Completed\r\n");

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());
    char c;
    QVERIFY(socketDevice.read(&c, sizeof(c)) == -1);
    QVERIFY(socketDevice.error() == QAbstractSocket::RemoteHostClosedError);
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

QTEST_MAIN(tst_QSocks5SocketEngine)
#include "tst_qsocks5socketengine.moc"
