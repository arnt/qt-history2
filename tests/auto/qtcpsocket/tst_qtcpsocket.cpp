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

#include <qplatformdefs.h>

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QHostInfo>
#include <QMap>
#include <QMessageBox>
#include <QPointer>
#include <QProcess>
#include <QPushButton>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QDebug>
#ifdef TEST_QNETWORK_PROXY
# include <QNetworkProxy>
#endif

#ifdef Q_OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

Q_DECLARE_METATYPE(QAbstractSocket::SocketError)   
Q_DECLARE_METATYPE(QAbstractSocket::SocketState)   

//TESTED_FILES=network/qtcpsocket.cpp network/qtcpsocket.h network/qabstractsocket.cpp network/qabstractsocket.h network/qabstractsocket_p.h

class QTcpSocket;

class tst_QTcpSocket : public QObject
{
    Q_OBJECT

public:
    tst_QTcpSocket();
    virtual ~tst_QTcpSocket();


public slots:
    void initTestCase_data();
    void init();
    void cleanup();
private slots:
    void constructing();
    void setInvalidSocketDescriptor();
    void setSocketDescriptor();
    void blockingIMAP();
    void nonBlockingIMAP();
    void hostNotFound();
    void timeoutConnect();
    void delayedClose();
    void ipv6Connect();
    void partialRead();
    void unget();
    void readAllAfterClose();
    void openCloseOpenClose();
    void downloadBigFile();
    void connectToMultiIP();
    void readLine();
    void readLineString();
    void readChunks();
    void waitForBytesWritten();
    void waitForReadyRead();
    void flush();
    void synchronousApi();
    void dontCloseOnTimeout();
    void recursiveReadyRead();
    void atEnd();
    void socketInAThread();
    void socketsInThreads();
    void waitForReadyReadInASlot();
    void remoteCloseError();
    void openMessageBoxInErrorSlot();
    void connectToLocalHostNoService();
    void waitForConnectedInHostLookupSlot();
    void waitForConnectedInHostLookupSlot2();
    void readyReadSignalsAfterWaitForReadyRead();
    void linuxKernelBugLocalSocket();
    void abortiveClose();
    void localAddressEmptyOnBSD();
    void readWriteFailsOnUnconnectedSocket();
    void hammerTest();
    void connectionRefused();
    void suddenRemoteDisconnect_data();
    void suddenRemoteDisconnect();

protected slots:
    void nonBlockingIMAP_hostFound();
    void nonBlockingIMAP_connected();
    void nonBlockingIMAP_closed();
    void nonBlockingIMAP_readyRead();
    void nonBlockingIMAP_bytesWritten(qint64);
    void readRegularFile_readyRead();
    void exitLoopSlot();
    void downloadBigFileSlot();
    void recursiveReadyReadSlot();
    void waitForReadyReadInASlotSlot();
    void messageBoxSlot();
    void hostLookupSlot();
    void abortiveClose_abortSlot();
    void remoteCloseErrorSlot();
    void hammerTestSlot();

private:
    QTcpSocket *nonBlockingIMAP_socket;
    QStringList nonBlockingIMAP_data;
    qint64 nonBlockingIMAP_totalWritten;

    QTcpSocket *tmpSocket;
    qint64 bytesAvailable;


    bool gotClosedSignal;
    int numConnections;
};

tst_QTcpSocket::tst_QTcpSocket()
{
    tmpSocket = 0;
}

tst_QTcpSocket::~tst_QTcpSocket()
{

}

void tst_QTcpSocket::initTestCase_data()
{
    QTest::addColumn<bool>("setProxy");
    QTest::addColumn<int>("proxyType");

    QTest::newRow("WithoutProxy") << false << 0;
#ifdef TEST_QNETWORK_PROXY
    QTest::newRow("WithSocks5Proxy") << true << int(QNetworkProxy::Socks5Proxy);
#endif
}

void tst_QTcpSocket::init()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QFETCH_GLOBAL(int, proxyType);
        if (proxyType == QNetworkProxy::Socks5Proxy) {
            QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy,
                                                             QHostInfo::fromName("fluke.troll.no").addresses().first().toString(), 1080));
        }
#endif
    }
}

void tst_QTcpSocket::cleanup()
{
    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy) {
#ifdef TEST_QNETWORK_PROXY
        QNetworkProxy::setApplicationProxy(QNetworkProxy::DefaultProxy);
#endif
    }
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::constructing()
{


    QTcpSocket socket;

    // Check the initial state of the QTcpSocket.
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
    QVERIFY(socket.isSequential());
    QVERIFY(!socket.isOpen());
    QVERIFY(!socket.isValid());
    QCOMPARE(socket.socketType(), QTcpSocket::TcpSocket);

    QCOMPARE((int) socket.bytesAvailable(), 0);
    QCOMPARE(socket.canReadLine(), false);
    QCOMPARE(socket.readLine(), QByteArray());
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE((int) socket.localPort(), 0);
    QVERIFY(socket.localAddress() == QHostAddress());
    QCOMPARE((int) socket.peerPort(), 0);
    QVERIFY(socket.peerAddress() == QHostAddress());
    QCOMPARE(socket.error(), QTcpSocket::UnknownSocketError);
    QCOMPARE(socket.errorString(), QString("Unknown error"));

    // Check the state of the socket layer?
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::setInvalidSocketDescriptor()
{
    QTcpSocket socket;
    QCOMPARE(socket.socketDescriptor(), -1);
    QVERIFY(!socket.setSocketDescriptor(-5, QTcpSocket::UnconnectedState));
    QCOMPARE(socket.socketDescriptor(), -1);

    QCOMPARE(socket.error(), QTcpSocket::UnsupportedSocketOperationError);
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::setSocketDescriptor()
{
#ifdef Q_OS_WIN
    // need the dummy to ensure winsock is started
    QTcpSocket dummy;
    dummy.connectToHost("imap.troll.no", 143);
    QVERIFY(dummy.waitForConnected());

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        qErrnoWarning(WSAGetLastError(), "INVALID_SOCKET");
    }
#else
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
#endif

    QVERIFY(sock != INVALID_SOCKET);
    QTcpSocket socket;
    QVERIFY(socket.setSocketDescriptor(sock, QTcpSocket::UnconnectedState));
    QCOMPARE(socket.socketDescriptor(), (int)sock);

    socket.connectToHost("imap.troll.no", 143);
    QCOMPARE(socket.state(), QTcpSocket::HostLookupState);
    QCOMPARE(socket.socketDescriptor(), (int)sock);
    QVERIFY(socket.waitForConnected(5000));
#ifdef TEST_QNETWORK_PROXY
    if (QNetworkProxy::applicationProxy().type() == QNetworkProxy::NoProxy)
        QCOMPARE(socket.socketDescriptor(), (int)sock);
#endif
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::blockingIMAP()
{
    QTcpSocket socket;

    // Connect
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForConnected());
    QCOMPARE(socket.state(), QTcpSocket::ConnectedState);
    QVERIFY(socket.isValid());

    // Read greeting
    QVERIFY(socket.waitForReadyRead(5000));
    QString s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");

    // Write NOOP
    QCOMPARE((int) socket.write("1 NOOP\r\n", 8), 8);
    QCOMPARE((int) socket.write("2 NOOP\r\n", 8), 8);

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    // Read response
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "1 OK Completed\r\n");

    // Write a third NOOP to verify that write doesn't clear the read buffer
    QCOMPARE((int) socket.write("3 NOOP\r\n", 8), 8);

    // Read second response
    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "2 OK Completed\r\n");

    // Read third response
    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "3 OK Completed\r\n");


    // Write LOGOUT
    QCOMPARE((int) socket.write("4 LOGOUT\r\n", 10), 10);

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    // Read two lines of respose
    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "* BYE LOGOUT received\r\n");

    if (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(5000));

    s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "4 OK Completed\r\n");

    // Close the socket
    socket.close();

    // Check that it's closed
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::hostNotFound()
{
    QTcpSocket socket;

    socket.connectToHost("nosuchserver.troll.no", 80);
    QVERIFY(!socket.waitForConnected());
    QCOMPARE(socket.error(), QTcpSocket::HostNotFoundError);
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::timeoutConnect()
{
    QTcpSocket socket;

    // Outgoing port 53 is firewalled in the Oslo office.
    socket.connectToHost("cisco.com", 53);
    QVERIFY(!socket.waitForConnected(5000));
    QCOMPARE(socket.error(), QTcpSocket::SocketTimeoutError);
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::nonBlockingIMAP()
{
    QTcpSocket socket;
    connect(&socket, SIGNAL(hostFound()), SLOT(nonBlockingIMAP_hostFound()));
    connect(&socket, SIGNAL(connected()), SLOT(nonBlockingIMAP_connected()));
    connect(&socket, SIGNAL(disconnected()), SLOT(nonBlockingIMAP_closed()));
    connect(&socket, SIGNAL(bytesWritten(qint64)), SLOT(nonBlockingIMAP_bytesWritten(qint64)));
    connect(&socket, SIGNAL(readyRead()), SLOT(nonBlockingIMAP_readyRead()));
    nonBlockingIMAP_socket = &socket;

    // Connect
    socket.connectToHost("imap.troll.no", 143);
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
    QVERIFY(!nonBlockingIMAP_data.isEmpty());
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(),
            "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");
    nonBlockingIMAP_data.clear();

    nonBlockingIMAP_totalWritten = 0;

    // Write NOOP
    QCOMPARE((int) socket.write("1 NOOP\r\n", 8), 8);


    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(nonBlockingIMAP_totalWritten == 8);


    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }


    // Read response
    QVERIFY(!nonBlockingIMAP_data.isEmpty());
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(), "1 OK Completed\r\n");
    nonBlockingIMAP_data.clear();


    nonBlockingIMAP_totalWritten = 0;

    // Write LOGOUT
    QCOMPARE((int) socket.write("2 LOGOUT\r\n", 10), 10);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(nonBlockingIMAP_totalWritten == 10);

    // Wait for greeting
    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        QFAIL("Timed out");
    }

    // Read two lines of respose
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(), "* BYE LOGOUT received\r\n");
    QCOMPARE(nonBlockingIMAP_data.at(1).toLatin1().constData(), "2 OK Completed\r\n");
    nonBlockingIMAP_data.clear();

    // Close the socket
    socket.close();

    // Check that it's closed
    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

void tst_QTcpSocket::nonBlockingIMAP_hostFound()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_connected()
{
    QTestEventLoop::instance().exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_readyRead()
{
    while (nonBlockingIMAP_socket->canReadLine())
        nonBlockingIMAP_data.append(nonBlockingIMAP_socket->readLine());

    QTestEventLoop::instance().exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_bytesWritten(qint64 written)
{
    nonBlockingIMAP_totalWritten += written;
    QTestEventLoop::instance().exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_closed()
{
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::delayedClose()
{
    QTcpSocket socket;
    connect(&socket, SIGNAL(connected()), SLOT(nonBlockingIMAP_connected()));
    connect(&socket, SIGNAL(disconnected()), SLOT(exitLoopSlot()));

    socket.connectToHost("imap.troll.no", 143);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");

    QCOMPARE(socket.state(), QTcpSocket::ConnectedState);

    QCOMPARE((int) socket.write("1 LOGOUT\r\n", 10), 10);

    // Add a huge bulk of data to be written after the logout
    // command. The server will shut down after receiving the LOGOUT,
    // so this data will not be read. But our close call should
    // schedule a delayed close because all the data can not be
    // written in one go.
    QCOMPARE((int) socket.write(QByteArray(100000, '\n'), 100000), 100000);

    socket.close();

    QCOMPARE((int) socket.state(), (int) QTcpSocket::ClosingState);

    QTestEventLoop::instance().enterLoop(10);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");

    QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::ipv6Connect()
{
    QSKIP("This test will work when we have an IPv6 net up", SkipSingle);

    QTcpServer server;
    if (!server.listen(QHostAddress("fe80::2e0:4cff:fefb:662a"), 0)) {
        //### change this to determin the current address.
        if (server.serverError() == QTcpSocket::SocketAddressNotAvailableError)
            QSKIP("This test only works on shusaku", SkipSingle);
        QVERIFY(server.serverError() == QTcpSocket::UnsupportedSocketOperationError
               || server.serverError() == QTcpSocket::SocketAddressNotAvailableError);
        return;
    }

    QTcpSocket socket;

    socket.connectToHost(server.serverAddress(), server.serverPort());
    QVERIFY(socket.waitForConnected(5000));
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::partialRead()
{
    QTcpSocket socket;
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(socket.state() == QTcpSocket::ConnectedState);
    char buf[512];

    QByteArray greeting = "* OK esparsett Cyrus IMAP4 v2.2.8 server ready";

    for (int i = 0; i < 10; i += 2) {
        while (socket.bytesAvailable() < 2)
            QVERIFY(socket.waitForReadyRead(10000));
        QVERIFY(socket.read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
    }
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::unget()
{
    QTcpSocket socket;
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(socket.state() == QTcpSocket::ConnectedState);
    char buf[512];

    QByteArray greeting = "* OK esparsett Cyrus IMAP4 v2.2.8 server ready";

    for (int i = 0; i < 10; i += 2) {
        while (socket.bytesAvailable() < 2)
            QVERIFY(socket.waitForReadyRead(10000));
        int bA = socket.bytesAvailable();
        QVERIFY(socket.read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
        QCOMPARE((int)socket.bytesAvailable(), bA - 2);
        socket.ungetChar(buf[1]);
        socket.ungetChar(buf[0]);
        QCOMPARE((int)socket.bytesAvailable(), bA);
        QVERIFY(socket.read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readRegularFile_readyRead()
{
    QTestEventLoop::instance().exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readAllAfterClose()
{
    QTcpSocket socket;
    socket.connectToHost("imap.troll.no", 143);
    connect(&socket, SIGNAL(readyRead()), SLOT(readRegularFile_readyRead()));
    QTestEventLoop::instance().enterLoop(10);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Network operation timed out");

    socket.close();
    QByteArray array = socket.readAll();
    QCOMPARE(array.size(), 0);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::openCloseOpenClose()
{
    QTcpSocket socket;

    for (int i = 0; i < 3; ++i) {
        QCOMPARE(socket.state(), QTcpSocket::UnconnectedState);
        QVERIFY(socket.isSequential());
        QVERIFY(!socket.isOpen());
        QVERIFY(socket.socketType() == QTcpSocket::TcpSocket);

        QCOMPARE((int) socket.bytesAvailable(), 0);
        QCOMPARE(socket.canReadLine(), false);
        QCOMPARE(socket.readLine(), QByteArray());
        QCOMPARE(socket.socketDescriptor(), -1);
        QCOMPARE((int) socket.localPort(), 0);
        QVERIFY(socket.localAddress() == QHostAddress());
        QCOMPARE((int) socket.peerPort(), 0);
        QVERIFY(socket.peerAddress() == QHostAddress());
        QCOMPARE(socket.error(), QTcpSocket::UnknownSocketError);
        QCOMPARE(socket.errorString(), QString("Unknown error"));

        QVERIFY(socket.state() == QTcpSocket::UnconnectedState);

        socket.connectToHost("imap.troll.no", 143);
        QVERIFY(socket.waitForConnected(5000));
        socket.close();
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::downloadBigFile()
{
    if (tmpSocket)
        delete tmpSocket;
    tmpSocket = new QTcpSocket;

    connect(tmpSocket, SIGNAL(connected()), SLOT(exitLoopSlot()));
    connect(tmpSocket, SIGNAL(readyRead()), SLOT(downloadBigFileSlot()));

    tmpSocket->connectToHost("ares.troll.no", 80);

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout()) {
        delete tmpSocket;
        tmpSocket = 0;
        QFAIL("Network operation timed out");
    }

    QVERIFY(tmpSocket->state() == QAbstractSocket::ConnectedState);
    QVERIFY(tmpSocket->write("GET /~ahanssen/QTest/mediumfile HTTP/1.0\r\n") > 0);
    QVERIFY(tmpSocket->write("HOST: ares.troll.no\r\n") > 0);
    QVERIFY(tmpSocket->write("\r\n") > 0);

    bytesAvailable = 0;

    QTime stopWatch;
    stopWatch.start();

    QTestEventLoop::instance().enterLoop(60);
    if (QTestEventLoop::instance().timeout()) {
        delete tmpSocket;
        tmpSocket = 0;
        QFAIL("Network operation timed out");
    }

    QCOMPARE(bytesAvailable, qint64(10000281));

    QVERIFY(tmpSocket->state() == QAbstractSocket::ConnectedState);

    qDebug("\t\t%.1fMB/%.1fs: %.1fMB/s",
           bytesAvailable / (1024.0 * 1024.0),
           stopWatch.elapsed() / 1024.0,
           (bytesAvailable / (stopWatch.elapsed() / 1000.0)) / (1024 * 1024));

    delete tmpSocket;
    tmpSocket = 0;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::exitLoopSlot()
{
    QTestEventLoop::instance().exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::downloadBigFileSlot()
{
    bytesAvailable += tmpSocket->readAll().size();
    if (bytesAvailable == 10000281)
        QTestEventLoop::instance().exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::connectToMultiIP()
{
    QTcpSocket socket;

    // rationale: this domain resolves to 5 A-records, 4 of them are
    // invalid. QTcpSocket should never spend more than 30 seconds per
    // IP, and 30s*5 = 150s. Allowing 10 seconds slack for processing.
    QTime stopWatch;
    stopWatch.start();
    socket.connectToHost("multi.andreas.hanssen.name", 80);
    QVERIFY(socket.waitForConnected(160000));
    QVERIFY(stopWatch.elapsed() < 170000);
    socket.abort();

    stopWatch.restart();
    socket.connectToHost("multi.andreas.hanssen.name", 81);
    QVERIFY(!socket.waitForConnected(10000));
    QVERIFY(stopWatch.elapsed() < 11000);
    QCOMPARE(socket.error(), QAbstractSocket::SocketTimeoutError);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readLine()
{
    QTcpSocket socket;
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForConnected(5000));

    while (!socket.canReadLine())
        QVERIFY(socket.waitForReadyRead(10000));

    char buffer[1024];
    QCOMPARE(socket.readLine(buffer, sizeof(buffer)), qint64(48));

    // * OK lupinella Cyrus IMAP4 v2.1.12 server ready__
    // 01234567890123456789012345678901234567890123456789
    QCOMPARE((int) buffer[46], (int) '\r');
    QCOMPARE((int) buffer[47], (int) '\n');
    QCOMPARE((int) buffer[48], (int) '\0');

    QCOMPARE(socket.write("1 NOOP\r\n"), qint64(8));

    while (socket.bytesAvailable() < 10)
        QVERIFY(socket.waitForReadyRead(10000));

    QCOMPARE(socket.readLine(buffer, 11), qint64(10));
    QCOMPARE((const char *)buffer, "1 OK Compl");

    while (socket.bytesAvailable() < 6)
        QVERIFY(socket.waitForReadyRead(10000));

    QCOMPARE(socket.readLine(buffer, 11), qint64(6));
    QCOMPARE((const char *)buffer, "eted\r\n");

    QVERIFY(!socket.waitForReadyRead(100));
    QCOMPARE(socket.readLine(buffer, sizeof(buffer)), qint64(-1));
    QVERIFY(socket.error() == QAbstractSocket::SocketTimeoutError
            || socket.error() == QAbstractSocket::RemoteHostClosedError);
    QCOMPARE(socket.bytesAvailable(), qint64(0));
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readLineString()
{
    QTcpSocket socket;
    QByteArray expected("* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForReadyRead(5000));

    QByteArray arr = socket.readLine();
    QCOMPARE(arr, expected);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readChunks()
{
    QTcpSocket socket;
    socket.connectToHost("imap.troll.no", 143);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(socket.waitForReadyRead(5000));

    char buf[4096];
    memset(buf, '@', sizeof(buf));
    qint64 dataLength = socket.read(buf, sizeof(buf));
    QVERIFY(dataLength > 0);

    QCOMPARE(buf[dataLength - 2], '\r');
    QCOMPARE(buf[dataLength - 1], '\n');
    QCOMPARE(buf[dataLength], '@');
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForBytesWritten()
{
    QTcpSocket socket;
    socket.connectToHost("shusaku.troll.no", 22);
    QVERIFY(socket.waitForConnected(5000));

    socket.write(QByteArray(10000, '@'));
    qint64 toWrite = socket.bytesToWrite();
    QVERIFY(socket.waitForBytesWritten(5000));
    QVERIFY(toWrite > socket.bytesToWrite());
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForReadyRead()
{
    QTcpSocket socket;
    socket.connectToHost("shusaku.troll.no", 22);
    socket.waitForReadyRead(0);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::flush()
{
    QTcpSocket socket;
    socket.flush();

    connect(&socket, SIGNAL(connected()), SLOT(exitLoopSlot()));
    socket.connectToHost("imap.troll.no", 143);
    QTestEventLoop::instance().enterLoop(5000);
    QVERIFY(socket.isOpen());

    socket.write("1 LOGOUT\r\n");
    QCOMPARE(socket.bytesToWrite(), qint64(10));
    socket.flush();
    QCOMPARE(socket.bytesToWrite(), qint64(0));
    socket.close();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::synchronousApi()
{
    QTcpSocket ftpSocket;
    ftpSocket.connectToHost("www.trolltech.com", 80);
    ftpSocket.write("GET / HTTP/1.0\r\n\r\n");
    QVERIFY(ftpSocket.waitForDisconnected());
    QVERIFY(ftpSocket.bytesAvailable() > 0);
    QByteArray arr = ftpSocket.readAll();
    QVERIFY(arr.size() > 0);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::dontCloseOnTimeout()
{
    QTcpServer server;
    QVERIFY(server.listen());

    QHostAddress serverAddress = QHostAddress::LocalHost;
    if (!(server.serverAddress() == QHostAddress::Any))
        serverAddress = server.serverAddress();

    QTcpSocket socket;
    socket.connectToHost(serverAddress, server.serverPort());
    QVERIFY(!socket.waitForReadyRead(100));
    QCOMPARE(socket.error(), QTcpSocket::SocketTimeoutError);
    QVERIFY(socket.isOpen());

    QVERIFY(!socket.waitForDisconnected(100));
    QCOMPARE(socket.error(), QTcpSocket::SocketTimeoutError);
    QVERIFY(socket.isOpen());

}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::recursiveReadyRead()
{
    QTcpSocket smtp;
    connect(&smtp, SIGNAL(connected()), SLOT(exitLoopSlot()));
    connect(&smtp, SIGNAL(readyRead()), SLOT(recursiveReadyReadSlot()));
    tmpSocket = &smtp;

    QSignalSpy spy(&smtp, SIGNAL(readyRead()));

    smtp.connectToHost("smtp.trolltech.com", 25);
    QTestEventLoop::instance().enterLoop(30);
    QVERIFY2(!QTestEventLoop::instance().timeout(),
            "Timed out when connecting to smtp.trolltech.com:25");

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY2(!QTestEventLoop::instance().timeout(),
            "Timed out when waiting for the readyRead() signal");

    QCOMPARE(spy.count(), 1);

}

void tst_QTcpSocket::recursiveReadyReadSlot()
{
    // make sure the server spits out more data
    tmpSocket->write("NOOP\r\n");
    tmpSocket->flush();

    // indiscriminately enter the event loop and start processing
    // events again. but oops! future socket notifications will cause
    // undesired recursive behavior. Unless QTcpSocket is smart, which
    // it of course is. :-)
    QEventLoop loop;
    for (int i = 0; i < 100; ++i)
        loop.processEvents();

    // all we really wanted to do was process some events, then exit
    // the loop
    QTestEventLoop::instance().exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::atEnd()
{
    QTcpSocket socket;
    socket.connectToHost("trueblue.troll.no", 21);

    QVERIFY(socket.waitForReadyRead(15000));
    QTextStream stream(&socket);
    QVERIFY(!stream.atEnd());
    QString greeting = stream.readLine();
    QVERIFY(stream.atEnd());
    QCOMPARE(greeting, QString("220 trueblue.troll.no FTP server (Version 6.5/OpenBSD, linux port 0.3.2) ready."));
}

class TestThread : public QThread
{
    Q_OBJECT

public:
    inline QByteArray data() const
    {
        return socketData;
    }

protected:
    inline void run()
    {
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(getData()), Qt::DirectConnection);
        connect(socket, SIGNAL(disconnected()), this, SLOT(quit()), Qt::DirectConnection);

        socket->connectToHost("trueblue.troll.no", 21);
        socket->write("QUIT\r\n");
        exec();

        delete socket;
    }

private slots:
    inline void getData()
    {
        socketData += socket->readAll();
    }

private:
    int exitCode;
    QTcpSocket *socket;
    QByteArray socketData;
};

//----------------------------------------------------------------------------------
void tst_QTcpSocket::socketInAThread()
{
    for (int i = 0; i < 3; ++i) {
        TestThread thread;
        thread.start();
        QVERIFY(thread.wait(15000));
        QCOMPARE(thread.data().constData(),
                QByteArray("220 trueblue.troll.no FTP server (Version 6.5/"
                           "OpenBSD, linux port 0.3.2) ready.\r\n221 Goodbye.\r\n").constData());
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::socketsInThreads()
{
    for (int i = 0; i < 3; ++i) {
        TestThread thread1;
        TestThread thread2;
        TestThread thread3;

        thread1.start();
        thread2.start();
        thread3.start();

        QVERIFY(thread2.wait(15000));
        QVERIFY(thread3.wait(15000));
        QVERIFY(thread1.wait(15000));

        QCOMPARE(thread1.data().constData(),
                QByteArray("220 trueblue.troll.no FTP server (Version 6.5/"
                           "OpenBSD, linux port 0.3.2) ready.\r\n221 Goodbye.\r\n").constData());
        QCOMPARE(thread2.data().constData(),
                QByteArray("220 trueblue.troll.no FTP server (Version 6.5/"
                           "OpenBSD, linux port 0.3.2) ready.\r\n221 Goodbye.\r\n").constData());
        QCOMPARE(thread3.data().constData(),
                QByteArray("220 trueblue.troll.no FTP server (Version 6.5/"
                           "OpenBSD, linux port 0.3.2) ready.\r\n221 Goodbye.\r\n").constData());
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForReadyReadInASlot()
{
    QTcpSocket socket;
    tmpSocket = &socket;
    connect(&socket, SIGNAL(connected()), this, SLOT(waitForReadyReadInASlotSlot()));

    socket.connectToHost("www.trolltech.com", 80);
    socket.write("GET / HTTP/1.0\r\n\r\n");

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

void tst_QTcpSocket::waitForReadyReadInASlotSlot()
{
    QVERIFY(tmpSocket->waitForReadyRead(5000));
    QTestEventLoop::instance().exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::remoteCloseError()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    connect(&server, SIGNAL(newConnection()), this, SLOT(exitLoopSlot()));

    QTcpSocket clientSocket;
    connect(&clientSocket, SIGNAL(readyRead()), this, SLOT(exitLoopSlot()));

    clientSocket.connectToHost(server.serverAddress(), server.serverPort());

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    if (!server.hasPendingConnections()) {
        QTestEventLoop::instance().enterLoop(30);
        QVERIFY(!QTestEventLoop::instance().timeout());
    }

    QVERIFY(server.hasPendingConnections());
    QTcpSocket *serverSocket = server.nextPendingConnection();
    connect(&clientSocket, SIGNAL(disconnected()), this, SLOT(exitLoopSlot()));

    serverSocket->write("Hello");

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(clientSocket.bytesAvailable(), qint64(5));

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    QSignalSpy errorSpy(&clientSocket, SIGNAL(error(QAbstractSocket::SocketError)));
    QSignalSpy disconnectedSpy(&clientSocket, SIGNAL(disconnected()));

    serverSocket->disconnectFromHost();

    tmpSocket = &clientSocket;
    connect(&clientSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(remoteCloseErrorSlot()));

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(clientSocket.error(), QAbstractSocket::RemoteHostClosedError);

    delete serverSocket;

    clientSocket.connectToHost(server.serverAddress(), server.serverPort());

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QVERIFY(server.hasPendingConnections());
    serverSocket = server.nextPendingConnection();
    serverSocket->disconnectFromHost();

    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!QTestEventLoop::instance().timeout());

    QCOMPARE(clientSocket.state(), QAbstractSocket::UnconnectedState);
}

void tst_QTcpSocket::remoteCloseErrorSlot()
{
    QCOMPARE(tmpSocket->state(), QAbstractSocket::ConnectedState);
}

void tst_QTcpSocket::messageBoxSlot()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    socket->deleteLater();
    QMessageBox box;
    QTimer::singleShot(500, &box, SLOT(close()));
    box.exec();

    QTimer::singleShot(500, this, SLOT(exitLoopSlot()));
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::openMessageBoxInErrorSlot()
{
    QTcpSocket *socket = new QTcpSocket;
    QPointer<QTcpSocket> p(socket);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(messageBoxSlot()));
    socket->connectToHost("imap.troll.no", 9999); // ConnectionRefusedError
    QTestEventLoop::instance().enterLoop(30);
    QVERIFY(!p);
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::connectToLocalHostNoService()
{
    // This test was created after we received a report that claimed
    // QTcpSocket would crash if trying to connect to "localhost" on a random
    // port with no service listening.
    QTcpSocket *socket = new QTcpSocket;
    socket->connectToHost("localhost", 31415); // no service running here, one suspects
    while(socket->state() == QTcpSocket::HostLookupState || socket->state() == QTcpSocket::ConnectingState)
        QTestEventLoop::instance().enterLoop(2);
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);
    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForConnectedInHostLookupSlot()
{
    // This test tries to reproduce the problem where waitForConnected() is
    // called at a point where the host lookup is already done. QTcpSocket
    // will try to abort the "pending lookup", but since it's already done and
    // the queued signal is already underway, we will receive the signal after
    // waitForConnected() has returned, and control goes back to the event
    // loop. When the signal has been received, the connection is torn down,
    // then reopened. Yikes. If we reproduce this by calling
    // waitForConnected() inside hostLookupSlot(), it will even crash.
    tmpSocket = new QTcpSocket;
    QEventLoop loop;
    connect(tmpSocket, SIGNAL(connected()), &loop, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QSignalSpy timerSpy(&timer, SIGNAL(timeout()));
    timer.start(10000);

    connect(tmpSocket, SIGNAL(hostFound()), this, SLOT(hostLookupSlot()));
    tmpSocket->connectToHost("imap.troll.no", 143);

    loop.exec();
    QCOMPARE(timerSpy.count(), 0);

    delete tmpSocket;
}

void tst_QTcpSocket::hostLookupSlot()
{
    // This will fail to cancel the pending signal
    QVERIFY(tmpSocket->waitForConnected(5000));
}

class Foo : public QObject
{
    Q_OBJECT
    QTcpSocket sock;
public:
    int count;

    inline Foo(QObject *parent = 0) : QObject(parent)
    {
        count = 0;
        connect(&sock, SIGNAL(connected()), this, SLOT(connectedToIt()));
    }

public slots:
    inline void connectedToIt()
    { count++; }

    inline void doIt()
    {
        sock.connectToHost("ifi.uio.no", 13);

#ifdef Q_OS_MAC
        pthread_yield_np();
#elif defined Q_OS_LINUX
        pthread_yield();
#endif
        sock.waitForConnected();
    }

    inline void exitLoop()
    {
        QTestEventLoop::instance().exitLoop();
    }
};

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForConnectedInHostLookupSlot2()
{
#if QT_VERSION < 0x040100
    QSKIP("Fixed in 4.1.", SkipSingle);
#endif
#ifndef Q_OS_UNIX
    QSKIP("Unix-only test", SkipSingle);
#endif

    Foo foo;
    QPushButton top("Go", 0);
    top.show();
    connect(&top, SIGNAL(clicked()), &foo, SLOT(doIt()));

    QTimer::singleShot(100, &top, SLOT(animateClick()));
    QTimer::singleShot(5000, &foo, SLOT(exitLoop()));

    QTestEventLoop::instance().enterLoop(30);
    if (QTestEventLoop::instance().timeout())
        QFAIL("Timed out");

    QCOMPARE(foo.count, 1);
}


//----------------------------------------------------------------------------------
void tst_QTcpSocket::readyReadSignalsAfterWaitForReadyRead()
{
#if QT_VERSION < 0x040101
    QSKIP("Fixed in 4.1.1", SkipSingle);
#endif

    QTcpSocket socket;

    QSignalSpy readyReadSpy(&socket, SIGNAL(readyRead()));

    // Connect
    socket.connectToHost("imap.troll.no", 143);

    // Wait for the read
    QVERIFY(socket.waitForReadyRead(5000));

    QCOMPARE(readyReadSpy.count(), 1);

    QString s = socket.readLine();
    QCOMPARE(s.toLatin1().constData(), "* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n");
    QCOMPARE(socket.bytesAvailable(), qint64(0));

    QTestEventLoop::instance().enterLoop(1);
    QCOMPARE(socket.bytesAvailable(), qint64(0));
    QCOMPARE(readyReadSpy.count(), 1);
}

class TestThread2 : public QThread
{
    Q_OBJECT
public:
    void run()
    {
        QFile fileWriter("fifo");
        QVERIFY(fileWriter.open(QFile::WriteOnly));
        QCOMPARE(fileWriter.write(QByteArray(32, '@')), qint64(32));
        QCOMPARE(fileWriter.write(QByteArray(32, '@')), qint64(32));
        QCOMPARE(fileWriter.write(QByteArray(32, '@')), qint64(32));
        QCOMPARE(fileWriter.write(QByteArray(32, '@')), qint64(32));
    }
};

//----------------------------------------------------------------------------------
void tst_QTcpSocket::linuxKernelBugLocalSocket()
{
#ifdef Q_OS_LINUX
    QFile::remove("fifo");
    mkfifo("fifo", 0666);

    TestThread2 test;
    test.start();

    QFile fileReader("fifo");
    QVERIFY(fileReader.open(QFile::ReadOnly));

    test.wait();

    QTcpSocket socket;
    socket.setSocketDescriptor(fileReader.handle());
    QVERIFY(socket.waitForReadyRead(5000));
    QCOMPARE(socket.bytesAvailable(), qint64(128));

    QFile::remove("fifo");
#else
    QSKIP("Linux-only test", SkipSingle);
#endif
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::abortiveClose()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    connect(&server, SIGNAL(newConnection()), this, SLOT(exitLoopSlot()));

    QTcpSocket clientSocket;
    clientSocket.connectToHost(server.serverAddress(), server.serverPort());

    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(server.hasPendingConnections());

    QVERIFY(tmpSocket = server.nextPendingConnection());
    
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    QSignalSpy readyReadSpy(&clientSocket, SIGNAL(readyRead()));
    QSignalSpy errorSpy(&clientSocket, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(&clientSocket, SIGNAL(disconnected()), this, SLOT(exitLoopSlot()));
    QTimer::singleShot(0, this, SLOT(abortiveClose_abortSlot()));

    QTestEventLoop::instance().enterLoop(5);

    QCOMPARE(readyReadSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);

    QCOMPARE(*static_cast<const int *>(errorSpy.at(0).at(0).constData()),
             int(QAbstractSocket::RemoteHostClosedError));

}

void tst_QTcpSocket::abortiveClose_abortSlot()
{
    tmpSocket->abort();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::localAddressEmptyOnBSD()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    // we try 10 times, but note that this doesn't always provoke the bug
    for (int i = 0; i < 10; ++i) {
        QTcpSocket tcpSocket;
        tcpSocket.connectToHost(QHostAddress::LocalHost, server.serverPort());
        if (!tcpSocket.waitForConnected(0)) {
            // to provoke the bug, we need a local socket that connects immediately
            // --i;
            tcpSocket.abort();
            if (tcpSocket.state() != QTcpSocket::UnconnectedState)
                QVERIFY(tcpSocket.waitForDisconnected(-1));
            continue;
        }
        QCOMPARE(tcpSocket.localAddress(), QHostAddress(QHostAddress::LocalHost));
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readWriteFailsOnUnconnectedSocket()
{
    QTcpSocket socket;
    socket.connectToHost("www.trolltech.com", 80);
    socket.write("GET / HTTP/1.0\r\n\r\n");
    QVERIFY(socket.waitForDisconnected(5000));
    QCOMPARE(socket.error(), QAbstractSocket::RemoteHostClosedError);

    char c[16];
    QCOMPARE(socket.write("BLUBBER"), qint64(-1));
    QVERIFY(!socket.readAll().isEmpty());
    QCOMPARE(socket.read(c, 16), qint64(0));
    QEXPECT_FAIL("", "socket.readLine() /should/ return 0 when there's nothing to read", Continue);
    QCOMPARE(socket.readLine(c, 16), qint64(0));
    QVERIFY(!socket.getChar(c));
    QVERIFY(!socket.putChar('a'));

    socket.close();

    QCOMPARE(socket.read(c, 16), qint64(-1));
    QCOMPARE(socket.readLine(c, 16), qint64(-1));
    QVERIFY(!socket.getChar(c));
    QVERIFY(!socket.putChar('a'));
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::hammerTest()
{
    QSKIP("This test is far too intensive, it stresses fluke so bad"
          " that it triggers random errors.", SkipAll);
    const int NumSockets = 100;

    QList<QTcpSocket *> sockets;
    for (int i = 0; i < NumSockets; ++i) {
        sockets << new QTcpSocket(this);
        connect(sockets.last(), SIGNAL(readyRead()), this, SLOT(hammerTestSlot()));
    }

    numConnections = 0;

    QTime stopWatch;
    stopWatch.start();

    foreach (QTcpSocket *socket, sockets)
        socket->connectToHost("fluke.troll.no", 21 /* ftp */);

    int timeout = 100;
    do {
        QTestEventLoop::instance().enterLoop(1);
    } while (numConnections < NumSockets && --timeout);

    int elapsed = stopWatch.elapsed();
    qDebug() << numConnections << "/" << (elapsed / 1000.0) << "secs ="
             << ((numConnections * 1000.0) / elapsed) << "connections/sec";

    foreach (QTcpSocket *socket, sockets)
        QCOMPARE(socket->state(), QAbstractSocket::ConnectedState);

    foreach (QTcpSocket *socket, sockets) {
        socket->write("QUIT\r\n");
        socket->disconnectFromHost();
    }

    foreach (QTcpSocket *socket, sockets)
        QVERIFY(socket->waitForDisconnected(5000));
}

void tst_QTcpSocket::hammerTestSlot()
{
    ++numConnections;
}

void tst_QTcpSocket::connectionRefused()
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    
    QTcpSocket socket;
    QSignalSpy stateSpy(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
    QSignalSpy errorSpy(&socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QEventLoop loop;
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), &loop, SLOT(quit()));
    QTimer::singleShot(5000, &loop, SLOT(quit()));

    socket.connectToHost("fluke.troll.no", 144);
    
    loop.exec();

    QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
    QCOMPARE(socket.error(), QAbstractSocket::ConnectionRefusedError);

    QCOMPARE(stateSpy.count(), 3);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(0).at(0)), QAbstractSocket::HostLookupState);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(1).at(0)), QAbstractSocket::ConnectingState);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(2).at(0)), QAbstractSocket::UnconnectedState);
    QCOMPARE(errorSpy.count(), 1);
}

void tst_QTcpSocket::suddenRemoteDisconnect_data()
{
    QTest::addColumn<QString>("client");
    QTest::addColumn<QString>("server");

    QTest::newRow("Qt3 Client <-> Qt3 Server") << QString::fromLatin1("qt3client") << QString::fromLatin1("qt3server");
    QTest::newRow("Qt3 Client <-> Qt4 Server") << QString::fromLatin1("qt3client") << QString::fromLatin1("qt4server");
    QTest::newRow("Qt4 Client <-> Qt3 Server") << QString::fromLatin1("qt4client") << QString::fromLatin1("qt3server");
    QTest::newRow("Qt4 Client <-> Qt4 Server") << QString::fromLatin1("qt4client") << QString::fromLatin1("qt4server");
}

void tst_QTcpSocket::suddenRemoteDisconnect()
{
    QFETCH(QString, client);
    QFETCH(QString, server);
    
    QFETCH_GLOBAL(int, proxyType);
    if (proxyType == QNetworkProxy::Socks5Proxy)
        return;

    // Start server
    QProcess serverProcess;
    serverProcess.setReadChannel(QProcess::StandardError);
    serverProcess.start(QString::fromLatin1("stressTest/stressTest %1").arg(server),
                        QIODevice::ReadWrite | QIODevice::Text);
    while (!serverProcess.canReadLine())
        QVERIFY(serverProcess.waitForReadyRead(10000));
    QCOMPARE(serverProcess.readLine().data(), (server.toLatin1() + "\n").data());

    // Start client
    QProcess clientProcess;
    clientProcess.setReadChannel(QProcess::StandardError);
    clientProcess.start(QString::fromLatin1("stressTest/stressTest %1").arg(client),
                        QIODevice::ReadWrite | QIODevice::Text);
    while (!clientProcess.canReadLine())
        QVERIFY(clientProcess.waitForReadyRead(10000));
    QCOMPARE(clientProcess.readLine().data(), (client.toLatin1() + "\n").data());

    // Let them play for a while
    qDebug("Running stress test");
    QEventLoop loop;
    connect(&serverProcess, SIGNAL(finished(int)), &loop, SLOT(quit()));
    connect(&clientProcess, SIGNAL(finished(int)), &loop, SLOT(quit()));
    QTime stopWatch;
    stopWatch.start();
    QTimer::singleShot(20000, &loop, SLOT(quit()));

    while ((serverProcess.state() == QProcess::Running
           || clientProcess.state() == QProcess::Running) && stopWatch.elapsed() < 20000)
        loop.exec();

    QVERIFY(stopWatch.elapsed() < 20000);

    // Check that both exited normally.
    QCOMPARE(clientProcess.readAll().constData(), "SUCCESS\n");
    QCOMPARE(serverProcess.readAll().constData(), "SUCCESS\n");
}

QTEST_MAIN(tst_QTcpSocket)
#include "tst_qtcpsocket.moc"
