/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>

#if QT_VERSION >= 0x040100

#ifdef Q_OS_WIN
#include <winsock2.h>
#endif

#include <qcoreapplication.h>


#include <qdatastream.h>
#include <private/qnativesocketengine_p.h>

#include <qhostaddress.h>
#include <qdatetime.h>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <stddef.h>


#include <qstringlist.h>

//TESTED_FILES=network/qnativesocketengine.cpp network/qnativesocketengine_p.h network/qnativesocketengine_unix.cpp

class tst_QNativeSocketEngine : public QObject
{
    Q_OBJECT

public:
    tst_QNativeSocketEngine();
    virtual ~tst_QNativeSocketEngine();


public slots:
    void init();
    void cleanup();
private slots:
    void construction();
    void simpleConnectToIMAP();
    void udpLoopbackTest();
    void udpIPv6LoopbackTest();
    void broadcastTest();
    void serverTest();
    void udpLoopbackPerformance();
    void tcpLoopbackPerformance();
    void readWriteBufferSize();
    void tooManySockets();
    void bind();
    void networkError();
    void setSocketDescriptor();
    void invalidSend();
    void receiveUrgentData();
};

static const char *IMAP_IP = "62.70.27.18";

tst_QNativeSocketEngine::tst_QNativeSocketEngine()
{
}

tst_QNativeSocketEngine::~tst_QNativeSocketEngine()
{

}

void tst_QNativeSocketEngine::init()
{
}

void tst_QNativeSocketEngine::cleanup()
{
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::construction()
{
    QNativeSocketEngine socketDevice;

    QVERIFY(!socketDevice.isValid());

    // Initialize device
    QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(socketDevice.isValid());
    QVERIFY(socketDevice.protocol() == QAbstractSocket::IPv4Protocol);
    QVERIFY(socketDevice.socketType() == QAbstractSocket::TcpSocket);
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
    QVERIFY(socketDevice.socketDescriptor() != -1);
    QVERIFY(socketDevice.localAddress() == QHostAddress());
    QVERIFY(socketDevice.localPort() == 0);
    QVERIFY(socketDevice.peerAddress() == QHostAddress());
    QVERIFY(socketDevice.peerPort() == 0);
    QVERIFY(socketDevice.error() == QAbstractSocket::UnknownSocketError);

    QTest::ignoreMessage(QtWarningMsg, "QNativeSocketEngine::bytesAvailable() was called in QAbstractSocket::UnconnectedState");
    QVERIFY(socketDevice.bytesAvailable() == 0);

    QTest::ignoreMessage(QtWarningMsg, "QNativeSocketEngine::hasPendingDatagrams() was called in QAbstractSocket::UnconnectedState");
    QVERIFY(!socketDevice.hasPendingDatagrams());
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::simpleConnectToIMAP()
{
    QNativeSocketEngine socketDevice;

    // Initialize device
    QVERIFY(socketDevice.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);

    // Connect to imap.trolltech.com's IP
    QVERIFY(!socketDevice.connectToHost(QHostAddress(IMAP_IP), 143));
    QVERIFY(socketDevice.state() == QAbstractSocket::ConnectingState);
    QVERIFY(socketDevice.waitForWrite());
    int guard = 100;
    do {
        if (socketDevice.connectToHost(QHostAddress(IMAP_IP), 143))
            break;
        QTest::qWait(100);
    } while (--guard && socketDevice.error() == QAbstractSocket::SocketError(11));
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
    QCOMPARE(array.constData(), "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");

    // Write a logout message
    QByteArray array2 = "ZZZ LOGOUT\r\n";
    QVERIFY(socketDevice.write(array2.data(),
                              array2.size()) == array2.size());

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());

    available = socketDevice.bytesAvailable();
    QVERIFY(available > 0);
    array.resize(available);
    QVERIFY(socketDevice.read(array.data(), array.size()) == available);

    // Check that the greeting is what we expect it to be
    QCOMPARE(array.constData(),
             "* BYE LOGOUT received\r\n"
             "ZZZ OK Completed\r\n");

    // Wait for the response
    QVERIFY(socketDevice.waitForRead());
    char c;
    QVERIFY(socketDevice.read(&c, sizeof(c)) == -1);
    QVERIFY(socketDevice.error() == QAbstractSocket::RemoteHostClosedError);
    QVERIFY(socketDevice.state() == QAbstractSocket::UnconnectedState);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::udpLoopbackTest()
{
    QNativeSocketEngine udpSocket;

    // Initialize device #1
    QVERIFY(udpSocket.initialize(QAbstractSocket::UdpSocket));
    QVERIFY(udpSocket.isValid());
    QVERIFY(udpSocket.socketDescriptor() != -1);
    QVERIFY(udpSocket.protocol() == QAbstractSocket::IPv4Protocol);
    QVERIFY(udpSocket.socketType() == QAbstractSocket::UdpSocket);
    QVERIFY(udpSocket.state() == QAbstractSocket::UnconnectedState);

    // Bind #1 to localhost
    QVERIFY(udpSocket.bind(QHostAddress("127.0.0.1"), 0));
    QVERIFY(udpSocket.state() == QAbstractSocket::BoundState);
    quint16 port = udpSocket.localPort();
    QVERIFY(port != 0);

    // Initialize device #2
    QNativeSocketEngine udpSocket2;
    QVERIFY(udpSocket2.initialize(QAbstractSocket::UdpSocket));

    // Connect device #2 to #1
    QVERIFY(udpSocket2.connectToHost(QHostAddress("127.0.0.1"), port));
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
    QVERIFY(senderAddress == QHostAddress("127.0.0.1"));
    QVERIFY(senderPort != 0);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::udpIPv6LoopbackTest()
{
    QNativeSocketEngine udpSocket;

    // Initialize device #1
    bool init = udpSocket.initialize(QAbstractSocket::UdpSocket, QAbstractSocket::IPv6Protocol);

    if (!init) {
        QVERIFY(udpSocket.error() == QAbstractSocket::UnsupportedSocketOperationError);
    } else {
        QVERIFY(udpSocket.protocol() == QAbstractSocket::IPv6Protocol);

        // Bind #1 to localhost
        QVERIFY(udpSocket.bind(QHostAddress("::1"), 0));
        QVERIFY(udpSocket.state() == QAbstractSocket::BoundState);
        quint16 port = udpSocket.localPort();
        QVERIFY(port != 0);

        // Initialize device #2
        QNativeSocketEngine udpSocket2;
        QVERIFY(udpSocket2.initialize(QAbstractSocket::UdpSocket, QAbstractSocket::IPv6Protocol));

        // Connect device #2 to #1
        QVERIFY(udpSocket2.connectToHost(QHostAddress("::1"), port));
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
        QVERIFY(senderAddress == QHostAddress("::1"));
        QVERIFY(senderPort != 0);
    }
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::broadcastTest()
{
#ifdef Q_OS_AIX
    QSKIP("Broadcast does not work on darko", SkipAll);
#endif
    QNativeSocketEngine broadcastSocket;

    // Initialize a regular Udp socket
    QVERIFY(broadcastSocket.initialize(QAbstractSocket::UdpSocket));

    // Bind to any port on all interfaces
    QVERIFY(broadcastSocket.bind(QHostAddress::Any, 0));
    QVERIFY(broadcastSocket.state() == QAbstractSocket::BoundState);
    quint16 port = broadcastSocket.localPort();
    QVERIFY(port > 0);

    // Broadcast an inappropriate troll message
    QByteArray trollMessage
        = "MOOT wtf is a MOOT? talk english not your sutpiD ENGLISH.";
    QVERIFY(broadcastSocket.writeDatagram(trollMessage.data(),
                                         trollMessage.size(),
                                         QHostAddress::Broadcast,
                                         port) == trollMessage.size());

    // Wait until we receive it ourselves
#if defined(Q_OS_FREEBSD)
    QEXPECT_FAIL("", "Broadcasting to 255.255.255.255 does not work on FreeBSD", Abort);
#endif
    QVERIFY(broadcastSocket.waitForRead());
    QVERIFY(broadcastSocket.hasPendingDatagrams());

    qlonglong available = broadcastSocket.pendingDatagramSize();
    QByteArray response;
    response.resize(available);
    QVERIFY(broadcastSocket.readDatagram(response.data(), response.size())
           == response.size());
    QCOMPARE(response, trollMessage);

}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::serverTest()
{
    QNativeSocketEngine server;

    // Initialize a Tcp socket
    QVERIFY(server.initialize(QAbstractSocket::TcpSocket));

    // Bind to any port on all interfaces
    QVERIFY(server.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(server.state() == QAbstractSocket::BoundState);
    quint16 port = server.localPort();

    // Listen for incoming connections
    QVERIFY(server.listen());
    QVERIFY(server.state() == QAbstractSocket::ListeningState);

    // Initialize a Tcp socket
    QNativeSocketEngine client;
    QVERIFY(client.initialize(QAbstractSocket::TcpSocket));
    if (!client.connectToHost(QHostAddress("127.0.0.1"), port)) {
        // Connect to our server
        int guard = 100;
        do {
            if (client.connectToHost(QHostAddress("127.0.0.1"), port))
                break;
            QTest::qWait(100);
        } while (--guard && client.error() == QAbstractSocket::SocketError(11));
    }

    // The server accepts the connection
    int socketDescriptor = server.accept();
    QVERIFY(socketDescriptor > 0);

    // A socket device is initialized on the server side, passing the
    // socket descriptor from accept(). It's pre-connected.
    QNativeSocketEngine serverSocket;
    QVERIFY(serverSocket.initialize(socketDescriptor));
    QVERIFY(serverSocket.state() == QAbstractSocket::ConnectedState);

    // The server socket sends a greeting to the clietn
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
void tst_QNativeSocketEngine::udpLoopbackPerformance()
{
    QNativeSocketEngine udpSocket;

    // Initialize device #1
    QVERIFY(udpSocket.initialize(QAbstractSocket::UdpSocket));
    QVERIFY(udpSocket.isValid());
    QVERIFY(udpSocket.socketDescriptor() != -1);
    QVERIFY(udpSocket.protocol() == QAbstractSocket::IPv4Protocol);
    QVERIFY(udpSocket.socketType() == QAbstractSocket::UdpSocket);
    QVERIFY(udpSocket.state() == QAbstractSocket::UnconnectedState);

    // Bind #1 to localhost
    QVERIFY(udpSocket.bind(QHostAddress("127.0.0.1"), 0));
    QVERIFY(udpSocket.state() == QAbstractSocket::BoundState);
    quint16 port = udpSocket.localPort();
    QVERIFY(port != 0);

    // Initialize device #2
    QNativeSocketEngine udpSocket2;
    QVERIFY(udpSocket2.initialize(QAbstractSocket::UdpSocket));

    // Connect device #2 to #1
    QVERIFY(udpSocket2.connectToHost(QHostAddress("127.0.0.1"), port));
    QVERIFY(udpSocket2.state() == QAbstractSocket::ConnectedState);

    const int messageSize = 8192;
    QByteArray message1(messageSize, '@');
    QByteArray answer(messageSize, '@');

    QHostAddress localhost = QHostAddress::LocalHost;

    qlonglong readBytes = 0;
    QTime timer;
    timer.start();
    while (timer.elapsed() < 5000) {
        udpSocket2.write(message1.data(), message1.size());
        udpSocket.waitForRead();
        while (udpSocket.hasPendingDatagrams()) {
            readBytes += (qlonglong) udpSocket.readDatagram(answer.data(),
                                                             answer.size());
        }
    }

    qDebug("\t\t%.1fMB/%.1fs: %.1fMB/s",
           readBytes / (1024.0 * 1024.0),
           timer.elapsed() / 1024.0,
           (readBytes / (timer.elapsed() / 1000.0)) / (1024 * 1024));
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::tcpLoopbackPerformance()
{
    QNativeSocketEngine server;

    // Initialize a Tcp socket
    QVERIFY(server.initialize(QAbstractSocket::TcpSocket));

    // Bind to any port on all interfaces
    QVERIFY(server.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(server.state() == QAbstractSocket::BoundState);
    quint16 port = server.localPort();
    
    // Listen for incoming connections
    QVERIFY(server.listen());
    QVERIFY(server.state() == QAbstractSocket::ListeningState);

    // Initialize a Tcp socket
    QNativeSocketEngine client;
    QVERIFY(client.initialize(QAbstractSocket::TcpSocket));

    // Connect to our server
    if (!client.connectToHost(QHostAddress("127.0.0.1"), port)) {
        int guard = 100;
        do {
            if (client.connectToHost(QHostAddress("127.0.0.1"), port))
                break;
            QTest::qWait(100);
        } while (--guard && client.error() == QAbstractSocket::SocketError(11));
    }

    // The server accepts the connectio
    int socketDescriptor = server.accept();
    QVERIFY(socketDescriptor > 0);

    // A socket device is initialized on the server side, passing the
    // socket descriptor from accept(). It's pre-connected.
    QNativeSocketEngine serverSocket;
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

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::readWriteBufferSize()
{
    QNativeSocketEngine device;

    QVERIFY(device.initialize(QAbstractSocket::TcpSocket));

    qint64 bufferSize = device.receiveBufferSize();
    QVERIFY(bufferSize != -1);
    device.setReceiveBufferSize(bufferSize + 1);
    QVERIFY(device.receiveBufferSize() > bufferSize);

    bufferSize = device.sendBufferSize();
    QVERIFY(bufferSize != -1);
    device.setSendBufferSize(bufferSize + 1);
    QVERIFY(device.sendBufferSize() > bufferSize);

}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::tooManySockets()
{
#if defined Q_OS_WIN
    QSKIP("Certain windows machines suffocate and spend too much time in this test.", SkipAll);
#endif
    QList<QNativeSocketEngine *> sockets;
    QNativeSocketEngine *socketLayer = 0;
    for (;;) {
        socketLayer = new QNativeSocketEngine;
        sockets.append(socketLayer);

        if (!socketLayer->initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol))
            break;
    }

    QCOMPARE(socketLayer->error(), QAbstractSocket::SocketResourceError);

    qDeleteAll(sockets);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::bind()
{
#ifndef Q_OS_WIN
    QNativeSocketEngine binder;
    QVERIFY(binder.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(!binder.bind(QHostAddress::Any, 82));
    QVERIFY(binder.error() == QAbstractSocket::SocketAccessError);
#endif

    QNativeSocketEngine binder2;
    QVERIFY(binder2.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(binder2.bind(QHostAddress::Any, 31180));

    QNativeSocketEngine binder3;
    QVERIFY(binder3.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));
    QVERIFY(!binder3.bind(QHostAddress::Any, 31180));
    QVERIFY(binder3.error() == QAbstractSocket::AddressInUseError);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::networkError()
{
    QNativeSocketEngine client;

    QVERIFY(client.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol));

    // Connect to imap.trolltech.com's IP
    QVERIFY(!client.connectToHost(QHostAddress(IMAP_IP), 143));
    QVERIFY(client.state() == QAbstractSocket::ConnectingState);
    QVERIFY(client.waitForWrite());
    int guard = 100;
    do {
        if (client.connectToHost(QHostAddress(IMAP_IP), 143))
            break;
        QTest::qWait(100);
    } while (--guard && client.error() == QAbstractSocket::SocketError(11));
    QVERIFY(client.state() == QAbstractSocket::ConnectedState);

    // An unexpected network error!
#ifdef Q_OS_WIN
    // could use shutdown to produce different errors
    ::closesocket(client.socketDescriptor());
#else
    ::close(client.socketDescriptor());
#endif

    QVERIFY(client.read(0, 0) == -1);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::setSocketDescriptor()
{
    QNativeSocketEngine socket1;
    QVERIFY(socket1.initialize(QAbstractSocket::TcpSocket));

    QNativeSocketEngine socket2;
    QVERIFY(socket2.initialize(socket1.socketDescriptor()));
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::invalidSend()
{
    QNativeSocketEngine socket;
    QVERIFY(socket.initialize(QAbstractSocket::TcpSocket));

    QTest::ignoreMessage(QtWarningMsg, "QNativeSocketEngine::writeDatagram() was"
                               " called by a socket other than QAbstractSocket::UdpSocket");
    QCOMPARE(socket.writeDatagram("hei", 3, QHostAddress::LocalHost, 143),
            (qlonglong) -1);
}

//---------------------------------------------------------------------------
void tst_QNativeSocketEngine::receiveUrgentData()
{
    QNativeSocketEngine server;

    QVERIFY(server.initialize(QAbstractSocket::TcpSocket));

    // Bind to any port on all interfaces
    QVERIFY(server.bind(QHostAddress("0.0.0.0"), 0));
    QVERIFY(server.state() == QAbstractSocket::BoundState);
    quint16 port = server.localPort();

    QVERIFY(server.listen());
    QVERIFY(server.state() == QAbstractSocket::ListeningState);

    QNativeSocketEngine client;
    QVERIFY(client.initialize(QAbstractSocket::TcpSocket));

    if (!client.connectToHost(QHostAddress("127.0.0.1"), port)) {
        int guard = 100;
        do {
            if (client.connectToHost(QHostAddress("127.0.0.1"), port))
                break;
            QTest::qWait(100);
        } while (--guard && client.error() == QAbstractSocket::SocketError(11));
    }

    int socketDescriptor = server.accept();
    QVERIFY(socketDescriptor > 0);

    QNativeSocketEngine serverSocket;
    QVERIFY(serverSocket.initialize(socketDescriptor));
    QVERIFY(serverSocket.state() == QAbstractSocket::ConnectedState);

    char msg;
    int available;
    QByteArray response;

    // The server sends an urgent message
    msg = 'Q';
    QCOMPARE(int(::send(socketDescriptor, &msg, sizeof(msg), MSG_OOB)), 1);

    // The client receives the urgent message
    QVERIFY(client.waitForRead());
    available = client.bytesAvailable();
    QCOMPARE(available, 1);
    response.resize(available);
    QCOMPARE(client.read(response.data(), response.size()), qint64(1));
    QCOMPARE(response.at(0), msg);

    // The client sends an urgent message
    msg = 'T';
    int clientDescriptor = client.socketDescriptor();
    QCOMPARE(int(::send(clientDescriptor, &msg, sizeof(msg), MSG_OOB)), 1);

    // The server receives the urgent message
    QVERIFY(serverSocket.waitForRead());
    available = serverSocket.bytesAvailable();
    QCOMPARE(available, 1);
    response.resize(available);
    QCOMPARE(serverSocket.read(response.data(), response.size()), qint64(1));
    QCOMPARE(response.at(0), msg);

}

QTEST_MAIN(tst_QNativeSocketEngine)
#include "tst_qnativesocketengine.moc"

#else // QT_VERSION
QTEST_NOOP_MAIN
#endif
