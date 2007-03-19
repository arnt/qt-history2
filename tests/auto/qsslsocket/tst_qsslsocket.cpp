/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <qsslsocket.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>

class tst_QSslSocket : public QObject
{
    Q_OBJECT

public:
    tst_QSslSocket();
    virtual ~tst_QSslSocket();

    static void enterLoop(int secs)
    {
        ++loopLevel;
        QTestEventLoop::instance().enterLoop(secs);
        --loopLevel;
    }

    static bool timeout()
    {
        return QTestEventLoop::instance().timeout();
    }

public slots:
    void initTestCase_data();
    void init();
    void cleanup();

#ifndef QT_NO_OPENSSL
private slots:
    void constructing();
    void simpleConnect();
    void simpleConnectWithIgnore();

    // API tests
    void addCaCertificate();
    void addCaCertificates();
    void addCaCertificates2();
    void caCertificates();
    void ciphers();
    void connectToHostEncrypted();
    void currentCipher();
    void flush();
    void isEncrypted();
    void localCertificate();
    void mode();
    void peerCertificate();
    void peerCertificateChain();
    void privateKey();
    void protocol();
    void resetCaCertificates();
    void resetCiphers();
    void setCaCertificates();
    void setCiphers();
    void setLocalCertificate();
    void setPrivateKey();
    void setProtocol();
    void setSocketDescriptor();
    void waitForEncrypted();
    void startClientHandShake();
    void startServerHandShake();
    void addGlobalCaCertificate();
    void addGlobalCaCertificates();
    void addGlobalCaCertificates2();
    void globalCaCertificates();
    void globalCiphers();
    void resetGlobalCiphers();
    void setGlobalCaCertificates();
    void setGlobalCiphers();
    void supportedCiphers();
    void supportsSsl();
    void systemCaCertificates();

    static void exitLoop()
    {
        // Safe exit - if we aren't in an event loop, don't
        // exit one.
        if (loopLevel > 0)
            QTestEventLoop::instance().exitLoop();
    }

protected slots:
    void ignoreErrorSlot()
    {
        socket->ignoreSslErrors();
    }

private:
    QSslSocket *socket;
#endif // QT_NO_OPENSSL
private:
    static int loopLevel;
};

int tst_QSslSocket::loopLevel = 0;

tst_QSslSocket::tst_QSslSocket()
{
#ifndef QT_NO_OPENSSL
    qRegisterMetaType<QList<QSslError> >("QList<QSslError>");
#endif
}

tst_QSslSocket::~tst_QSslSocket()
{

}

void tst_QSslSocket::initTestCase_data()
{
}

void tst_QSslSocket::init()
{
}

void tst_QSslSocket::cleanup()
{
}

#ifndef QT_NO_OPENSSL

void tst_QSslSocket::constructing()
{
    QSslSocket socket;
    QCOMPARE(socket.state(), QSslSocket::UnconnectedState);
    QCOMPARE(socket.mode(), QSslSocket::PlainMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(socket.bytesAvailable(), qint64(0));
    QCOMPARE(socket.bytesToWrite(), qint64(0));
    QVERIFY(!socket.canReadLine());
    QVERIFY(socket.atEnd());
    QCOMPARE(socket.localCertificate(), QSslCertificate());
    QCOMPARE(socket.errorString(), QString("Unknown error"));
    char c = '\0';
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device");
    QVERIFY(!socket.getChar(&c));
    QCOMPARE(c, '\0');
    QVERIFY(!socket.isOpen());
    QVERIFY(!socket.isReadable());
    QVERIFY(socket.isSequential());
    QVERIFY(!socket.isTextModeEnabled());
    QVERIFY(!socket.isWritable());
    QCOMPARE(socket.openMode(), QIODevice::NotOpen);
    QVERIFY(socket.peek(2).isEmpty());
    QCOMPARE(socket.pos(), qint64(0));
    QVERIFY(!socket.putChar('c'));
    QVERIFY(socket.read(2).isEmpty());
    QCOMPARE(socket.read(0, 0), qint64(-1));
    QVERIFY(socket.readAll().isEmpty());
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::readLine: Called with maxSize < 2");
    QCOMPARE(socket.readLine(0, 0), qint64(-1));
    char buf[10];
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::getChar: Closed device"); // readLine is based on getChar
    QCOMPARE(socket.readLine(buf, sizeof(buf)), qint64(-1));
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::seek: The device is not open");
    QVERIFY(!socket.reset());
    QTest::ignoreMessage(QtWarningMsg, "QIODevice::seek: The device is not open");
    QVERIFY(!socket.seek(2));
    QCOMPARE(socket.size(), qint64(0));
    QVERIFY(!socket.waitForBytesWritten(10));
    QVERIFY(!socket.waitForReadyRead(10));
    QCOMPARE(socket.write(0, 0), qint64(-1));
    QCOMPARE(socket.write(QByteArray()), qint64(-1));
    QCOMPARE(socket.error(), QAbstractSocket::UnknownSocketError);
    QVERIFY(!socket.flush());
    QVERIFY(!socket.isValid());
    QCOMPARE(socket.localAddress(), QHostAddress());
    QCOMPARE(socket.localPort(), quint16(0));
    QCOMPARE(socket.peerAddress(), QHostAddress());
    QVERIFY(socket.peerName().isEmpty());
    QCOMPARE(socket.peerPort(), quint16(0));
    QCOMPARE(socket.proxy().type(), QNetworkProxy::DefaultProxy);
    QCOMPARE(socket.readBufferSize(), qint64(0));
    QCOMPARE(socket.socketDescriptor(), -1);
    QCOMPARE(socket.socketType(), QAbstractSocket::TcpSocket);
    QVERIFY(!socket.waitForConnected(10));
    QTest::ignoreMessage(QtWarningMsg, "QSslSocket::waitForDisconnected() is not allowed in UnconnectedState");
    QVERIFY(!socket.waitForDisconnected(10));
    QCOMPARE(socket.protocol(), QSslSocket::SslV3);
}

void tst_QSslSocket::simpleConnect()
{
    QSslSocket socket;
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy hostFoundSpy(&socket, SIGNAL(hostFound()));
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy connectionEncryptedSpy(&socket, SIGNAL(encrypted()));
    QSignalSpy sslErrorsSpy(&socket, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&socket, SIGNAL(connected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(modeChanged(QSslSocket::Mode)), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(encrypted()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(exitLoop()));

    // Start connecting
    socket.connectToHost("imap.troll.no", 993);
    QCOMPARE(socket.state(), QAbstractSocket::HostLookupState);
    enterLoop(10);

    // Entered connecting state
    QCOMPARE(socket.state(), QAbstractSocket::ConnectingState);
    QCOMPARE(connectedSpy.count(), 0);
    QCOMPARE(hostFoundSpy.count(), 1);
    QCOMPARE(disconnectedSpy.count(), 0);
    enterLoop(10);

    // Entered connected state
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(socket.mode(), QSslSocket::PlainMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(hostFoundSpy.count(), 1);
    QCOMPARE(disconnectedSpy.count(), 0);

    // Enter encrypted mode
    socket.startClientHandShake();
    QCOMPARE(socket.mode(), QSslSocket::SslClientMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(connectionEncryptedSpy.count(), 0);
    QCOMPARE(sslErrorsSpy.count(), 0);

    // Starting handshake
    enterLoop(10);
    QCOMPARE(sslErrorsSpy.count(), 1);
    QCOMPARE(connectionEncryptedSpy.count(), 0);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
}

void tst_QSslSocket::simpleConnectWithIgnore()
{
    QSslSocket socket;
    this->socket = &socket;
    QSignalSpy encryptedSpy(&socket, SIGNAL(encrypted()));
    QSignalSpy sslErrorsSpy(&socket, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&socket, SIGNAL(readyRead()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(encrypted()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(connected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(exitLoop()));

    // Start connecting
    socket.connectToHost("imap.troll.no", 993);
    QCOMPARE(socket.state(), QAbstractSocket::HostLookupState);
    enterLoop(10);

    // Start handshake
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    socket.startClientHandShake();
    enterLoop(10);

    // Done; encryption should be enabled.
    QCOMPARE(sslErrorsSpy.count(), 1);
    QVERIFY(socket.isEncrypted());
    QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
    QCOMPARE(encryptedSpy.count(), 1);

    // Wait for incoming data
    if (!socket.canReadLine())
        enterLoop(10);

    QCOMPARE(socket.readAll(), QByteArray("* OK esparsett Cyrus IMAP4 v2.2.8 server ready\r\n"));

    socket.disconnectFromHost();
}

void tst_QSslSocket::addCaCertificate()
{
}

void tst_QSslSocket::addCaCertificates()
{
}

void tst_QSslSocket::addCaCertificates2()
{
}

void tst_QSslSocket::caCertificates()
{
}

void tst_QSslSocket::ciphers()
{
}

void tst_QSslSocket::connectToHostEncrypted()
{
}

void tst_QSslSocket::currentCipher()
{
}

void tst_QSslSocket::flush()
{
}

void tst_QSslSocket::isEncrypted()
{
}

void tst_QSslSocket::localCertificate()
{
}

void tst_QSslSocket::mode()
{
}

void tst_QSslSocket::peerCertificate()
{
}

void tst_QSslSocket::peerCertificateChain()
{
}

void tst_QSslSocket::privateKey()
{
}

void tst_QSslSocket::protocol()
{
}

void tst_QSslSocket::resetCaCertificates()
{
}

void tst_QSslSocket::resetCiphers()
{
}

void tst_QSslSocket::setCaCertificates()
{
}

void tst_QSslSocket::setCiphers()
{
}

void tst_QSslSocket::setLocalCertificate()
{
}

void tst_QSslSocket::setPrivateKey()
{
}

void tst_QSslSocket::setProtocol()
{
}

void tst_QSslSocket::setSocketDescriptor()
{
}

void tst_QSslSocket::waitForEncrypted()
{
    QSslSocket socket;
    this->socket = &socket;

    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    socket.connectToHostEncrypted("imap.troll.no", 993);

    QVERIFY(socket.waitForEncrypted(10000));
}

void tst_QSslSocket::startClientHandShake()
{
}

void tst_QSslSocket::startServerHandShake()
{
}

void tst_QSslSocket::addGlobalCaCertificate()
{
}

void tst_QSslSocket::addGlobalCaCertificates()
{
}

void tst_QSslSocket::addGlobalCaCertificates2()
{
}

void tst_QSslSocket::globalCaCertificates()
{
}

void tst_QSslSocket::globalCiphers()
{
}

void tst_QSslSocket::resetGlobalCiphers()
{
}

void tst_QSslSocket::setGlobalCaCertificates()
{
}

void tst_QSslSocket::setGlobalCiphers()
{
}

void tst_QSslSocket::supportedCiphers()
{
}

void tst_QSslSocket::supportsSsl()
{
}

void tst_QSslSocket::systemCaCertificates()
{
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslSocket)
#include "tst_qsslsocket.moc"
