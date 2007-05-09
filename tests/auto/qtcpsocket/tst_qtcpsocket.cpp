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
#include <sys/types.h>
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
#ifndef QT_NO_OPENSSL
#include <QSslSocket>
#endif
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QDebug>
#ifndef TEST_QNETWORK_PROXY
#define TEST_QNETWORK_PROXY
#endif
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

    static void enterLoop(int secs)
    {
        ++loopLevel;
        QTestEventLoop::instance().enterLoop(secs);
        --loopLevel;
    }
    static void exitLoop()
    {
        // Safe exit - if we aren't in an event loop, don't
        // exit one.
        if (loopLevel > 0)
            QTestEventLoop::instance().exitLoop();
    }
    static bool timeout()
    {
        return QTestEventLoop::instance().timeout();
    }

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
    void partialRead();
    void unget();
    void readAllAfterClose();
    void openCloseOpenClose();
    void downloadBigFile();
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
#ifndef Q_OS_WIN
    void connectToLocalHostNoService();
#endif
    void waitForConnectedInHostLookupSlot();
#ifndef Q_OS_WIN
    void waitForConnectedInHostLookupSlot2();
#endif
    void readyReadSignalsAfterWaitForReadyRead();
#ifdef Q_OS_LINUX
    void linuxKernelBugLocalSocket();
#endif
    void abortiveClose();
    void localAddressEmptyOnBSD();
    void readWriteFailsOnUnconnectedSocket();
    void connectionRefused();
    void suddenRemoteDisconnect_data();
    void suddenRemoteDisconnect();
    void connectToMultiIP();
    void moveToThread0();

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

private:
    QTcpSocket *newSocket() const;
    QTcpSocket *nonBlockingIMAP_socket;
    QStringList nonBlockingIMAP_data;
    qint64 nonBlockingIMAP_totalWritten;

    QTcpSocket *tmpSocket;
    qint64 bytesAvailable;
    qint64 expectedLength;
    bool readingBody;
    QTime timer;


    bool gotClosedSignal;
    int numConnections;
    static int loopLevel;
};

int tst_QTcpSocket::loopLevel = 0;

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
    QTest::addColumn<bool>("ssl");

    QTest::newRow("WithoutProxy") << false << 0 << false;
#ifdef TEST_QNETWORK_PROXY
    QTest::newRow("WithSocks5Proxy") << true << int(QNetworkProxy::Socks5Proxy) << false;
    QTest::newRow("WithHttpProxy") << true << int(QNetworkProxy::HttpProxy) << false;
#endif
#ifndef QT_NO_OPENSSL
    QTest::newRow("WithoutProxy SSL") << false << 0 << true;
#ifdef TEST_QNETWORK_PROXY
    QTest::newRow("WithSocks5Proxy SSL") << true << int(QNetworkProxy::Socks5Proxy) << true;
    QTest::newRow("WithHttpProxy SSL") << true << int(QNetworkProxy::HttpProxy) << true;
#endif
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
        } else if (proxyType == QNetworkProxy::HttpProxy) {
            QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy,
                                                             QHostInfo::fromName("fluke.troll.no").addresses().first().toString(), 3128));
        }
#endif
    }
}

QTcpSocket *tst_QTcpSocket::newSocket() const
{
#ifndef QT_NO_OPENSSL
    QFETCH_GLOBAL(bool, ssl);
    return ssl ? new QSslSocket : new QTcpSocket;
#else
    return new QTcpSocket;
#endif
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
    QTcpSocket *socket = newSocket();

    // Check the initial state of the QTcpSocket.
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);
    QVERIFY(socket->isSequential());
    QVERIFY(!socket->isOpen());
    QVERIFY(!socket->isValid());
    QCOMPARE(socket->socketType(), QTcpSocket::TcpSocket);

    QCOMPARE((int) socket->bytesAvailable(), 0);
    QCOMPARE(socket->canReadLine(), false);
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
    QCOMPARE(socket->readLine(), QByteArray());
    QCOMPARE(socket->socketDescriptor(), -1);
    QCOMPARE((int) socket->localPort(), 0);
    QVERIFY(socket->localAddress() == QHostAddress());
    QCOMPARE((int) socket->peerPort(), 0);
    QVERIFY(socket->peerAddress() == QHostAddress());
    QCOMPARE(socket->error(), QTcpSocket::UnknownSocketError);
    QCOMPARE(socket->errorString(), QString("Unknown error"));

    // Check the state of the socket layer?
    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::setInvalidSocketDescriptor()
{
    QTcpSocket *socket = newSocket();
    QCOMPARE(socket->socketDescriptor(), -1);
    QVERIFY(!socket->setSocketDescriptor(-5, QTcpSocket::UnconnectedState));
    QCOMPARE(socket->socketDescriptor(), -1);

    QCOMPARE(socket->error(), QTcpSocket::UnsupportedSocketOperationError);

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::setSocketDescriptor()
{
#ifdef Q_OS_WIN
    // need the dummy to ensure winsock is started
    QTcpSocket *dummy = newSocket();
    dummy->connectToHost("fluke.troll.no", 143);
    QVERIFY(dummy->waitForConnected());

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        qErrnoWarning(WSAGetLastError(), "INVALID_SOCKET");
    }
#else
    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
#endif

    QVERIFY(sock != INVALID_SOCKET);
    QTcpSocket *socket = newSocket();
    QVERIFY(socket->setSocketDescriptor(sock, QTcpSocket::UnconnectedState));
    QCOMPARE(socket->socketDescriptor(), (int)sock);

    socket->connectToHost("fluke.troll.no", 143);
    QCOMPARE(socket->state(), QTcpSocket::HostLookupState);
    QCOMPARE(socket->socketDescriptor(), (int)sock);
    QVERIFY(socket->waitForConnected(10000));
#ifdef TEST_QNETWORK_PROXY
    if (QNetworkProxy::applicationProxy().type() == QNetworkProxy::NoProxy)
        QCOMPARE(socket->socketDescriptor(), (int)sock);
#endif
    delete socket;
#ifdef Q_OS_WIN
    delete dummy;
#endif
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::blockingIMAP()
{
    QTcpSocket *socket = newSocket();

    // Connect
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForConnected(10000));
    QCOMPARE(socket->state(), QTcpSocket::ConnectedState);
    QVERIFY(socket->isValid());

    // Read greeting
    QVERIFY(socket->waitForReadyRead(5000));
    QString s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "* OK fluke Cyrus IMAP4 v2.2.12 server ready\r\n");

    // Write NOOP
    QCOMPARE((int) socket->write("1 NOOP\r\n", 8), 8);
    QCOMPARE((int) socket->write("2 NOOP\r\n", 8), 8);

    if (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(5000));

    // Read response
    s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "1 OK Completed\r\n");

    // Write a third NOOP to verify that write doesn't clear the read buffer
    QCOMPARE((int) socket->write("3 NOOP\r\n", 8), 8);

    // Read second response
    if (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(5000));
    s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "2 OK Completed\r\n");

    // Read third response
    if (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(5000));
    s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "3 OK Completed\r\n");


    // Write LOGOUT
    QCOMPARE((int) socket->write("4 LOGOUT\r\n", 10), 10);

    if (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(5000));

    // Read two lines of respose
    s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "* BYE LOGOUT received\r\n");

    if (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(5000));

    s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "4 OK Completed\r\n");

    // Close the socket
    socket->close();

    // Check that it's closed
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::hostNotFound()
{
    QTcpSocket *socket = newSocket();

    socket->connectToHost("nosuchserver.troll.no", 80);
    QVERIFY(!socket->waitForConnected());
    QCOMPARE(socket->error(), QTcpSocket::HostNotFoundError);
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::timeoutConnect()
{
    QTcpSocket *socket = newSocket();

    // Outgoing port 53 is firewalled in the Oslo office.
    socket->connectToHost("cisco.com", 53);
    QVERIFY(!socket->waitForConnected(100));
    QCOMPARE(socket->error(), QTcpSocket::SocketTimeoutError);
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::nonBlockingIMAP()
{
    QTcpSocket *socket = newSocket();
    connect(socket, SIGNAL(hostFound()), SLOT(nonBlockingIMAP_hostFound()));
    connect(socket, SIGNAL(connected()), SLOT(nonBlockingIMAP_connected()));
    connect(socket, SIGNAL(disconnected()), SLOT(nonBlockingIMAP_closed()));
    connect(socket, SIGNAL(bytesWritten(qint64)), SLOT(nonBlockingIMAP_bytesWritten(qint64)));
    connect(socket, SIGNAL(readyRead()), SLOT(nonBlockingIMAP_readyRead()));
    nonBlockingIMAP_socket = socket;

    // Connect
    socket->connectToHost("fluke.troll.no", 143);
    QCOMPARE(socket->state(), QTcpSocket::HostLookupState);

    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }

    if (socket->state() == QTcpSocket::ConnectingState) {
        enterLoop(30);
        if (timeout()) {
            QFAIL("Timed out");
        }
    }

    QCOMPARE(socket->state(), QTcpSocket::ConnectedState);

    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }

    // Read greeting
    QVERIFY(!nonBlockingIMAP_data.isEmpty());
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(),
            "* OK fluke Cyrus IMAP4 v2.2.12 server ready\r\n");
    nonBlockingIMAP_data.clear();

    nonBlockingIMAP_totalWritten = 0;

    // Write NOOP
    QCOMPARE((int) socket->write("1 NOOP\r\n", 8), 8);


    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(nonBlockingIMAP_totalWritten == 8);


    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }


    // Read response
    QVERIFY(!nonBlockingIMAP_data.isEmpty());
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(), "1 OK Completed\r\n");
    nonBlockingIMAP_data.clear();


    nonBlockingIMAP_totalWritten = 0;

    // Write LOGOUT
    QCOMPARE((int) socket->write("2 LOGOUT\r\n", 10), 10);

    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }

    QVERIFY(nonBlockingIMAP_totalWritten == 10);

    // Wait for greeting
    enterLoop(30);
    if (timeout()) {
        QFAIL("Timed out");
    }

    // Read two lines of respose
    QCOMPARE(nonBlockingIMAP_data.at(0).toLatin1().constData(), "* BYE LOGOUT received\r\n");
    QCOMPARE(nonBlockingIMAP_data.at(1).toLatin1().constData(), "2 OK Completed\r\n");
    nonBlockingIMAP_data.clear();

    // Close the socket
    socket->close();

    // Check that it's closed
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);

    delete socket;
}

void tst_QTcpSocket::nonBlockingIMAP_hostFound()
{
    exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_connected()
{
    exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_readyRead()
{
    while (nonBlockingIMAP_socket->canReadLine())
        nonBlockingIMAP_data.append(nonBlockingIMAP_socket->readLine());

    exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_bytesWritten(qint64 written)
{
    nonBlockingIMAP_totalWritten += written;
    exitLoop();
}

void tst_QTcpSocket::nonBlockingIMAP_closed()
{
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::delayedClose()
{
    QTcpSocket *socket = newSocket();
    connect(socket, SIGNAL(connected()), SLOT(nonBlockingIMAP_connected()));
    connect(socket, SIGNAL(disconnected()), SLOT(exitLoopSlot()));

    socket->connectToHost("fluke.troll.no", 143);

    enterLoop(30);
    if (timeout())
        QFAIL("Timed out");

    QCOMPARE(socket->state(), QTcpSocket::ConnectedState);

    QCOMPARE((int) socket->write("1 LOGOUT\r\n", 10), 10);

    // Add a huge bulk of data to be written after the logout
    // command. The server will shut down after receiving the LOGOUT,
    // so this data will not be read. But our close call should
    // schedule a delayed close because all the data can not be
    // written in one go.
    QCOMPARE((int) socket->write(QByteArray(100000, '\n'), 100000), 100000);

    socket->close();

    QCOMPARE((int) socket->state(), (int) QTcpSocket::ClosingState);

    enterLoop(10);
    if (timeout())
        QFAIL("Timed out");

    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::partialRead()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForConnected(10000));
    QVERIFY(socket->state() == QTcpSocket::ConnectedState);
    char buf[512];

    QByteArray greeting = "* OK fluke Cyrus IMAP4 v2.2.12 server ready";

    for (int i = 0; i < 10; i += 2) {
        while (socket->bytesAvailable() < 2)
            QVERIFY(socket->waitForReadyRead(5000));
        QVERIFY(socket->read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
    }

    delete socket;
}

//----------------------------------------------------------------------------------

void tst_QTcpSocket::unget()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForConnected(10000));
    QVERIFY(socket->state() == QTcpSocket::ConnectedState);
    char buf[512];

    QByteArray greeting = "* OK fluke Cyrus IMAP4 v2.2.12 server ready";

    for (int i = 0; i < 10; i += 2) {
        while (socket->bytesAvailable() < 2)
            QVERIFY(socket->waitForReadyRead(5000));
        int bA = socket->bytesAvailable();
        QVERIFY(socket->read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
        QCOMPARE((int)socket->bytesAvailable(), bA - 2);
        socket->ungetChar(buf[1]);
        socket->ungetChar(buf[0]);
        QCOMPARE((int)socket->bytesAvailable(), bA);
        QVERIFY(socket->read(buf, 2) == 2);
        buf[2] = '\0';
        QCOMPARE((char *)buf, greeting.mid(i, 2).data());
    }

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readRegularFile_readyRead()
{
    exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readAllAfterClose()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 143);
    connect(socket, SIGNAL(readyRead()), SLOT(readRegularFile_readyRead()));
    enterLoop(10);
    if (timeout())
        QFAIL("Network operation timed out");

    socket->close();
    QByteArray array = socket->readAll();
    QCOMPARE(array.size(), 0);

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::openCloseOpenClose()
{
    QTcpSocket *socket = newSocket();

    for (int i = 0; i < 3; ++i) {
        QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);
        QVERIFY(socket->isSequential());
        QVERIFY(!socket->isOpen());
        QVERIFY(socket->socketType() == QTcpSocket::TcpSocket);

        QCOMPARE((int) socket->bytesAvailable(), 0);
        QCOMPARE(socket->canReadLine(), false);
        QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
        QCOMPARE(socket->readLine(), QByteArray());
        QCOMPARE(socket->socketDescriptor(), -1);
        QCOMPARE((int) socket->localPort(), 0);
        QVERIFY(socket->localAddress() == QHostAddress());
        QCOMPARE((int) socket->peerPort(), 0);
        QVERIFY(socket->peerAddress() == QHostAddress());
        QCOMPARE(socket->error(), QTcpSocket::UnknownSocketError);
        QCOMPARE(socket->errorString(), QString("Unknown error"));

        QVERIFY(socket->state() == QTcpSocket::UnconnectedState);

        socket->connectToHost("fluke.troll.no", 143);
        QVERIFY(socket->waitForConnected(10000));
        socket->close();
    }

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::downloadBigFile()
{
    if (tmpSocket)
        delete tmpSocket;
    tmpSocket = newSocket();

    connect(tmpSocket, SIGNAL(connected()), SLOT(exitLoopSlot()));
    connect(tmpSocket, SIGNAL(readyRead()), SLOT(downloadBigFileSlot()));

    tmpSocket->connectToHost("ares.troll.no", 80);

    enterLoop(30);
    if (timeout()) {
        delete tmpSocket;
        tmpSocket = 0;
        QFAIL("Network operation timed out");
    }

    QVERIFY(tmpSocket->state() == QAbstractSocket::ConnectedState);
    QVERIFY(tmpSocket->write("GET /~ahanssen/QTest/mediumfile HTTP/1.0\r\n") > 0);
    QVERIFY(tmpSocket->write("HOST: ares.troll.no\r\n") > 0);
    QVERIFY(tmpSocket->write("\r\n") > 0);

    bytesAvailable = 0;
    expectedLength = 0;
    readingBody = false;

    QTime stopWatch;
    stopWatch.start();

    enterLoop(600);
    if (timeout()) {
        delete tmpSocket;
        tmpSocket = 0;
        QFAIL("Network operation timed out");
    }

    QCOMPARE(bytesAvailable, expectedLength);

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
    exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::downloadBigFileSlot()
{
    if (!readingBody) {
        while (tmpSocket->canReadLine()) {
            QByteArray array = tmpSocket->readLine();
            if (array.startsWith("Content-Length"))
                expectedLength = array.simplified().split(' ').at(1).toInt();
            if (array == "\r\n") {
                readingBody = true;
                break;
            }
        }
    }
    if (readingBody) {
        bytesAvailable += tmpSocket->readAll().size();
        if (bytesAvailable == expectedLength)
            exitLoop();
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readLine()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForConnected(5000));

    while (!socket->canReadLine())
        QVERIFY(socket->waitForReadyRead(10000));

    char buffer[1024];
    QCOMPARE(socket->readLine(buffer, sizeof(buffer)), qint64(45));

    // * OK fluke Cyrus IMAP4 v2.2.12 server ready__
    // 01234567890123456789012345678901234567890123456789
    QCOMPARE((int) buffer[43], (int) '\r');
    QCOMPARE((int) buffer[44], (int) '\n');
    QCOMPARE((int) buffer[45], (int) '\0');

    QCOMPARE(socket->write("1 NOOP\r\n"), qint64(8));

    while (socket->bytesAvailable() < 10)
        QVERIFY(socket->waitForReadyRead(10000));

    QCOMPARE(socket->readLine(buffer, 11), qint64(10));
    QCOMPARE((const char *)buffer, "1 OK Compl");

    while (socket->bytesAvailable() < 6)
        QVERIFY(socket->waitForReadyRead(10000));

    QCOMPARE(socket->readLine(buffer, 11), qint64(6));
    QCOMPARE((const char *)buffer, "eted\r\n");

    QVERIFY(!socket->waitForReadyRead(100));
    QCOMPARE(socket->readLine(buffer, sizeof(buffer)), qint64(-1));
    QVERIFY(socket->error() == QAbstractSocket::SocketTimeoutError
            || socket->error() == QAbstractSocket::RemoteHostClosedError);
    QCOMPARE(socket->bytesAvailable(), qint64(0));

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readLineString()
{
    QTcpSocket *socket = newSocket();
    QByteArray expected("* OK fluke Cyrus IMAP4 v2.2.12 server ready\r\n");
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForReadyRead(10000));

    QByteArray arr = socket->readLine();
    QCOMPARE(arr, expected);

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readChunks()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 143);
    QVERIFY(socket->waitForConnected(10000));
    QVERIFY(socket->waitForReadyRead(5000));

    char buf[4096];
    memset(buf, '@', sizeof(buf));
    qint64 dataLength = socket->read(buf, sizeof(buf));
    QVERIFY(dataLength > 0);

    QCOMPARE(buf[dataLength - 2], '\r');
    QCOMPARE(buf[dataLength - 1], '\n');
    QCOMPARE(buf[dataLength], '@');

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForBytesWritten()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 22);
    QVERIFY(socket->waitForConnected(10000));

    socket->write(QByteArray(10000, '@'));
    qint64 toWrite = socket->bytesToWrite();
    QVERIFY(socket->waitForBytesWritten(5000));
    QVERIFY(toWrite > socket->bytesToWrite());

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForReadyRead()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 22);
    socket->waitForReadyRead(0);
    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::flush()
{
    QTcpSocket *socket = newSocket();
    socket->flush();

    connect(socket, SIGNAL(connected()), SLOT(exitLoopSlot()));
    socket->connectToHost("fluke.troll.no", 143);
    enterLoop(5000);
    QVERIFY(socket->isOpen());

    socket->write("1 LOGOUT\r\n");
    QCOMPARE(socket->bytesToWrite(), qint64(10));
    socket->flush();
    QCOMPARE(socket->bytesToWrite(), qint64(0));
    socket->close();

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::synchronousApi()
{
    QTcpSocket *ftpSocket = newSocket();
    ftpSocket->connectToHost("fluke.troll.no", 21);
    ftpSocket->write("QUIT\r\n");
    QVERIFY(ftpSocket->waitForDisconnected(10000));
    QVERIFY(ftpSocket->bytesAvailable() > 0);
    QByteArray arr = ftpSocket->readAll();
    QVERIFY(arr.size() > 0);
    delete ftpSocket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::dontCloseOnTimeout()
{
    QTcpServer server;
    QVERIFY(server.listen());

    QHostAddress serverAddress = QHostAddress::LocalHost;
    if (!(server.serverAddress() == QHostAddress::Any))
        serverAddress = server.serverAddress();

    QTcpSocket *socket = newSocket();
    socket->connectToHost(serverAddress, server.serverPort());
    QVERIFY(!socket->waitForReadyRead(100));
    QCOMPARE(socket->error(), QTcpSocket::SocketTimeoutError);
    QVERIFY(socket->isOpen());

    QVERIFY(!socket->waitForDisconnected(100));
    QCOMPARE(socket->error(), QTcpSocket::SocketTimeoutError);
    QVERIFY(socket->isOpen());

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::recursiveReadyRead()
{
    QTcpSocket *smtp = newSocket();
    connect(smtp, SIGNAL(connected()), SLOT(exitLoopSlot()));
    connect(smtp, SIGNAL(readyRead()), SLOT(recursiveReadyReadSlot()));
    tmpSocket = smtp;

    QSignalSpy spy(smtp, SIGNAL(readyRead()));

    smtp->connectToHost("smtp.trolltech.com", 25);
    enterLoop(30);
    QVERIFY2(!timeout(),
            "Timed out when connecting to smtp.trolltech.com:25");

    enterLoop(30);
    QVERIFY2(!timeout(),
            "Timed out when waiting for the readyRead() signal");

    QCOMPARE(spy.count(), 1);

    delete smtp;
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
    exitLoop();
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::atEnd()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 21);

    QVERIFY(socket->waitForReadyRead(15000));
    QTextStream stream(socket);
    QVERIFY(!stream.atEnd());
    QString greeting = stream.readLine();
    QVERIFY(stream.atEnd());
    QCOMPARE(greeting, QString("220 (vsFTPd 2.0.4)"));

    delete socket;
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
#ifndef QT_NO_OPENSSL
        QFETCH_GLOBAL(bool, ssl);
        if (ssl)
            socket = new QSslSocket;
        else
#endif
        socket = new QTcpSocket;
        connect(socket, SIGNAL(readyRead()), this, SLOT(getData()), Qt::DirectConnection);
        connect(socket, SIGNAL(disconnected()), this, SLOT(closed()), Qt::DirectConnection);

        socket->connectToHost("fluke.troll.no", 21);
        socket->write("QUIT\r\n");
        exec();

        delete socket;
    }

private slots:
    inline void getData()
    {
        socketData += socket->readAll();
    }

    inline void closed()
    {
        quit();
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
        QCOMPARE(thread.data(),
                 QByteArray("220 (vsFTPd 2.0.4)\r\n221 Goodbye.\r\n"));
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

        QCOMPARE(thread1.data(),
                 QByteArray("220 (vsFTPd 2.0.4)\r\n221 Goodbye.\r\n"));
        QCOMPARE(thread2.data(),
                 QByteArray("220 (vsFTPd 2.0.4)\r\n221 Goodbye.\r\n"));
        QCOMPARE(thread3.data(),
                 QByteArray("220 (vsFTPd 2.0.4)\r\n221 Goodbye.\r\n"));
    }
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::waitForReadyReadInASlot()
{
    QTcpSocket *socket = newSocket();
    tmpSocket = socket;
    connect(socket, SIGNAL(connected()), this, SLOT(waitForReadyReadInASlotSlot()));

    socket->connectToHost("fluke.troll.no", 80);
    socket->write("GET / HTTP/1.0\r\n\r\n");

    enterLoop(30);
    QVERIFY(!timeout());

    delete socket;
}

void tst_QTcpSocket::waitForReadyReadInASlotSlot()
{
    QVERIFY(tmpSocket->waitForReadyRead(10000));
    exitLoop();
}

class RemoteCloseErrorServer : public QTcpServer
{
    Q_OBJECT
public:
    RemoteCloseErrorServer()
    {
        connect(this, SIGNAL(newConnection()),
                this, SLOT(getConnection()));
    }

private slots:
    void getConnection()
    {
        tst_QTcpSocket::exitLoop();
    }
};

//----------------------------------------------------------------------------------
void tst_QTcpSocket::remoteCloseError()
{
    RemoteCloseErrorServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QCoreApplication::instance()->processEvents();

    QTcpSocket *clientSocket = newSocket();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(exitLoopSlot()));

    clientSocket->connectToHost(server.serverAddress(), server.serverPort());

    enterLoop(30);
    QVERIFY(!timeout());

    QVERIFY(server.hasPendingConnections());
    QTcpSocket *serverSocket = server.nextPendingConnection();
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(exitLoopSlot()));

    serverSocket->write("Hello");

    enterLoop(30);
    QVERIFY(!timeout());

    QCOMPARE(clientSocket->bytesAvailable(), qint64(5));

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    QSignalSpy errorSpy(clientSocket, SIGNAL(error(QAbstractSocket::SocketError)));
    QSignalSpy disconnectedSpy(clientSocket, SIGNAL(disconnected()));

    serverSocket->disconnectFromHost();

    tmpSocket = clientSocket;
    connect(clientSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(remoteCloseErrorSlot()));

    enterLoop(30);
    QVERIFY(!timeout());

    QCOMPARE(disconnectedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(clientSocket->error(), QAbstractSocket::RemoteHostClosedError);

    delete serverSocket;

    clientSocket->connectToHost(server.serverAddress(), server.serverPort());

    enterLoop(30);
    QVERIFY(!timeout());

    QVERIFY(server.hasPendingConnections());
    serverSocket = server.nextPendingConnection();
    serverSocket->disconnectFromHost();

    enterLoop(30);
    QVERIFY(!timeout());

    QCOMPARE(clientSocket->state(), QAbstractSocket::UnconnectedState);

    delete clientSocket;
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
    QTimer::singleShot(100, &box, SLOT(close()));

    // This should not delete the socket
    box.exec();

    // Fire a non-0 singleshot to leave time for the delete
    QTimer::singleShot(250, this, SLOT(exitLoopSlot()));
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::openMessageBoxInErrorSlot()
{
    QTcpSocket *socket = newSocket();
    QPointer<QTcpSocket> p(socket);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(messageBoxSlot()));

    socket->connectToHost("hostnotfoundhostnotfound.troll.no", 9999); // Host not found, fyi
    enterLoop(30);
    QVERIFY(!p);
}

//----------------------------------------------------------------------------------
#ifndef Q_OS_WIN
void tst_QTcpSocket::connectToLocalHostNoService()
{
    // This test was created after we received a report that claimed
    // QTcpSocket would crash if trying to connect to "localhost" on a random
    // port with no service listening.
    QTcpSocket *socket = newSocket();
    socket->connectToHost("localhost", 31415); // no service running here, one suspects

    while(socket->state() == QTcpSocket::HostLookupState || socket->state() == QTcpSocket::ConnectingState) {
        QTest::qWait(100);
    }
    QCOMPARE(socket->state(), QTcpSocket::UnconnectedState);
    delete socket;
}
#endif

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
    tmpSocket = newSocket();
    QEventLoop loop;
    connect(tmpSocket, SIGNAL(connected()), &loop, SLOT(quit()));
    QTimer timer;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QSignalSpy timerSpy(&timer, SIGNAL(timeout()));
    timer.start(15000);

    connect(tmpSocket, SIGNAL(hostFound()), this, SLOT(hostLookupSlot()));
    tmpSocket->connectToHost("fluke.troll.no", 143);

    loop.exec();
    QCOMPARE(timerSpy.count(), 0);

    delete tmpSocket;
}

void tst_QTcpSocket::hostLookupSlot()
{
    // This will fail to cancel the pending signal
    QVERIFY(tmpSocket->waitForConnected(10000));
}

class Foo : public QObject
{
    Q_OBJECT
    QTcpSocket *sock;
public:
    int count;

    inline Foo(QObject *parent = 0) : QObject(parent)
    {
        count = 0;
#ifndef QT_NO_OPENSSL
        QFETCH_GLOBAL(bool, ssl);
        if (ssl)
            sock = new QSslSocket;
        else
#endif
        sock = new QTcpSocket;
        connect(sock, SIGNAL(connected()), this, SLOT(connectedToIt()));
    }

    inline ~Foo()
    {
        delete sock;
    }

public slots:
    inline void connectedToIt()
    { count++; }

    inline void doIt()
    {
        sock->connectToHost("fluke.troll.no", 80);

#ifdef Q_OS_MAC
        pthread_yield_np();
#elif defined Q_OS_LINUX
        pthread_yield();
#endif
        sock->waitForConnected();
        tst_QTcpSocket::exitLoop();
    }

    inline void exitLoop()
    {
        tst_QTcpSocket::exitLoop();
    }
};

//----------------------------------------------------------------------------------
#ifndef Q_OS_WIN
void tst_QTcpSocket::waitForConnectedInHostLookupSlot2()
{

    Foo foo;
    QPushButton top("Go", 0);
    top.show();
    connect(&top, SIGNAL(clicked()), &foo, SLOT(doIt()));

    QTimer::singleShot(100, &top, SLOT(animateClick()));
    QTimer::singleShot(1000, &foo, SLOT(exitLoop()));

    enterLoop(30);
    if (timeout())
        QFAIL("Timed out");

    QCOMPARE(foo.count, 1);
}
#endif

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readyReadSignalsAfterWaitForReadyRead()
{
    QTcpSocket *socket = newSocket();

    QSignalSpy readyReadSpy(socket, SIGNAL(readyRead()));

    // Connect
    socket->connectToHost("fluke.troll.no", 143);

    // Wait for the read
    QVERIFY(socket->waitForReadyRead(10000));

    QCOMPARE(readyReadSpy.count(), 1);

    QString s = socket->readLine();
    QCOMPARE(s.toLatin1().constData(), "* OK fluke Cyrus IMAP4 v2.2.12 server ready\r\n");
    QCOMPARE(socket->bytesAvailable(), qint64(0));

    QCoreApplication::instance()->processEvents();
    QCOMPARE(socket->bytesAvailable(), qint64(0));
    QCOMPARE(readyReadSpy.count(), 1);

    delete socket;
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
#ifdef Q_OS_LINUX
void tst_QTcpSocket::linuxKernelBugLocalSocket()
{
    QFile::remove("fifo");
    mkfifo("fifo", 0666);

    TestThread2 test;
    test.start();

    QFile fileReader("fifo");
    QVERIFY(fileReader.open(QFile::ReadOnly));

    test.wait();

    QTcpSocket *socket = newSocket();
    socket->setSocketDescriptor(fileReader.handle());
    QVERIFY(socket->waitForReadyRead(5000));
    QCOMPARE(socket->bytesAvailable(), qint64(128));

    QFile::remove("fifo");

    delete socket;
}
#endif

//----------------------------------------------------------------------------------
void tst_QTcpSocket::abortiveClose()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));
    connect(&server, SIGNAL(newConnection()), this, SLOT(exitLoopSlot()));

    QTcpSocket *clientSocket = newSocket();
    clientSocket->connectToHost(server.serverAddress(), server.serverPort());

    enterLoop(10);
    QVERIFY(server.hasPendingConnections());

    QVERIFY(tmpSocket = server.nextPendingConnection());

    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    QSignalSpy readyReadSpy(clientSocket, SIGNAL(readyRead()));
    QSignalSpy errorSpy(clientSocket, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(exitLoopSlot()));
    QTimer::singleShot(0, this, SLOT(abortiveClose_abortSlot()));

    enterLoop(5);

    QCOMPARE(readyReadSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);

    QCOMPARE(*static_cast<const int *>(errorSpy.at(0).at(0).constData()),
             int(QAbstractSocket::RemoteHostClosedError));

    delete clientSocket;
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

    QTcpSocket *tcpSocket = 0;
    // we try 10 times, but note that this doesn't always provoke the bug
    for (int i = 0; i < 10; ++i) {
        delete tcpSocket;
        tcpSocket = newSocket();
        tcpSocket->connectToHost(QHostAddress::LocalHost, server.serverPort());
        if (!tcpSocket->waitForConnected(0)) {
            // to provoke the bug, we need a local socket that connects immediately
            // --i;
            tcpSocket->abort();
            if (tcpSocket->state() != QTcpSocket::UnconnectedState)
                QVERIFY(tcpSocket->waitForDisconnected(-1));
            continue;
        }
        QCOMPARE(tcpSocket->localAddress(), QHostAddress(QHostAddress::LocalHost));
    }
    delete tcpSocket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::readWriteFailsOnUnconnectedSocket()
{
    QTcpSocket *socket = newSocket();
    socket->connectToHost("fluke.troll.no", 80);
    socket->write("GET / HTTP/1.0\r\n\r\n");
    QVERIFY(socket->waitForDisconnected(10000));
    QCOMPARE(socket->error(), QAbstractSocket::RemoteHostClosedError);

    char c[16];
    QCOMPARE(socket->write("BLUBBER"), qint64(-1));
    QVERIFY(!socket->readAll().isEmpty());
    QCOMPARE(socket->read(c, 16), qint64(0));
    QEXPECT_FAIL("", "socket.readLine() /should/ return 0 when there's nothing to read", Continue);
    QCOMPARE(socket->readLine(c, 16), qint64(0));
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
    QVERIFY(!socket->getChar(c));
    QVERIFY(!socket->putChar('a'));

    socket->close();

    QCOMPARE(socket->read(c, 16), qint64(-1));
    QCOMPARE(socket->readLine(c, 16), qint64(-1));
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
    QVERIFY(!socket->getChar(c));
    QVERIFY(!socket->putChar('a'));

    delete socket;
}

void tst_QTcpSocket::connectionRefused()
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");

    QTcpSocket *socket = newSocket();
    QSignalSpy stateSpy(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
    QSignalSpy errorSpy(socket, SIGNAL(error(QAbstractSocket::SocketError)));
    QEventLoop loop;
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), &loop, SLOT(quit()));
    QTimer::singleShot(10000, &loop, SLOT(quit()));

    socket->connectToHost("fluke.troll.no", 144);

    loop.exec();

    QCOMPARE(socket->state(), QAbstractSocket::UnconnectedState);
    QCOMPARE(socket->error(), QAbstractSocket::ConnectionRefusedError);

    QCOMPARE(stateSpy.count(), 3);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(0).at(0)), QAbstractSocket::HostLookupState);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(1).at(0)), QAbstractSocket::ConnectingState);
    QCOMPARE(qVariantValue<QAbstractSocket::SocketState>(stateSpy.at(2).at(0)), QAbstractSocket::UnconnectedState);
    QCOMPARE(errorSpy.count(), 1);

    delete socket;
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

    QFETCH_GLOBAL(bool, setProxy);
    if (setProxy)
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
    qDebug("Running stress test for 5 seconds");
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

//----------------------------------------------------------------------------------
void tst_QTcpSocket::connectToMultiIP()
{
    qDebug("Please wait, this test can take a while...");

    QTcpSocket *socket = newSocket();
    // rationale: this domain resolves to 3 A-records, 2 of them are
    // invalid. QTcpSocket should never spend more than 30 seconds per IP, and
    // 30s*2 = 60s.
    QTime stopWatch;
    stopWatch.start();
    socket->connectToHost("multi.andreas.hanssen.name", 80);
    QVERIFY(socket->waitForConnected(60000));
    QVERIFY(stopWatch.elapsed() < 70000);
    socket->abort();

    stopWatch.restart();
    socket->connectToHost("multi.andreas.hanssen.name", 81);
    QVERIFY(!socket->waitForConnected(1000));
    QVERIFY(stopWatch.elapsed() < 2000);
    QCOMPARE(socket->error(), QAbstractSocket::SocketTimeoutError);

    delete socket;
}

//----------------------------------------------------------------------------------
void tst_QTcpSocket::moveToThread0()
{
    {
        // Case 1: Moved after connecting, before waiting for connection.
        QTcpSocket socket;
        socket.connectToHost("fluke.troll.no", 143);
        socket.moveToThread(0);
        QVERIFY(socket.waitForConnected(1000));
        socket.write("XXX LOGOUT\r\n");
        QVERIFY(socket.waitForBytesWritten(5000));
        QVERIFY(socket.waitForDisconnected());
    }
    {
        // Case 2: Moved before connecting
        QTcpSocket socket;
        socket.moveToThread(0);
        socket.connectToHost("fluke.troll.no", 143);
        QVERIFY(socket.waitForConnected(1000));
        socket.write("XXX LOGOUT\r\n");
        QVERIFY(socket.waitForBytesWritten(5000));
        QVERIFY(socket.waitForDisconnected());
    }
    {
        // Case 3: Moved after writing, while waiting for bytes to be written.
        QTcpSocket socket;
        socket.connectToHost("fluke.troll.no", 143);
        QVERIFY(socket.waitForConnected(1000));
        socket.write("XXX LOGOUT\r\n");
        socket.moveToThread(0);
        QVERIFY(socket.waitForBytesWritten(5000));
        QVERIFY(socket.waitForDisconnected());
    }
    {
        // Case 4: Moved after writing, while waiting for response.
        QTcpSocket socket;
        socket.connectToHost("fluke.troll.no", 143);
        QVERIFY(socket.waitForConnected(1000));
        socket.write("XXX LOGOUT\r\n");
        QVERIFY(socket.waitForBytesWritten(5000));
        socket.moveToThread(0);
        QVERIFY(socket.waitForDisconnected());
    }
}

QTEST_MAIN(tst_QTcpSocket)
#include "tst_qtcpsocket.moc"
