/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qnetworkproxy.h>
#include <QtNetwork/qsslcipher.h>
#include <QtNetwork/qsslkey.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qtcpserver.h>
#include <QtTest/QtTest>

Q_DECLARE_METATYPE(QAbstractSocket::SocketState)
Q_DECLARE_METATYPE(QAbstractSocket::SocketError)
#ifndef QT_NO_OPENSSL
Q_DECLARE_METATYPE(QSslSocket::SslMode)
#endif

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
    void ciphers();
    void connectToHostEncrypted();
    void sessionCipher();
    void flush();
    void isEncrypted();
    void localCertificate();
    void mode();
    void peerCertificate();
    void peerCertificateChain();
    void privateKey();
    void protocol();
    void setCaCertificates();
    void setLocalCertificate();
    void setPrivateKey();
    void setProtocol();
    void setSocketDescriptor();
    void waitForEncrypted();
    void waitForConnectedEncryptedReadyRead();
    void startClientEncryption();
    void startServerEncryption();
    void addDefaultCaCertificate();
    void addDefaultCaCertificates();
    void addDefaultCaCertificates2();
    void defaultCaCertificates();
    void defaultCiphers();
    void resetDefaultCiphers();
    void setDefaultCaCertificates();
    void setDefaultCiphers();
    void supportedCiphers();
    void systemCaCertificates();
    void wildcard();

    static void exitLoop()
    {
        // Safe exit - if we aren't in an event loop, don't
        // exit one.
        if (loopLevel > 0) {
            --loopLevel;
            QTestEventLoop::instance().exitLoop();
        }
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
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QSslSocket::SslMode");
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
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;

    QCOMPARE(socket.state(), QSslSocket::UnconnectedState);
    QCOMPARE(socket.mode(), QSslSocket::UnencryptedMode);
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
    QCOMPARE(socket.protocol(), QSsl::SslV3);
}

void tst_QSslSocket::simpleConnect()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    QSignalSpy connectedSpy(&socket, SIGNAL(connected()));
    QSignalSpy hostFoundSpy(&socket, SIGNAL(hostFound()));
    QSignalSpy disconnectedSpy(&socket, SIGNAL(disconnected()));
    QSignalSpy connectionEncryptedSpy(&socket, SIGNAL(encrypted()));
    QSignalSpy sslErrorsSpy(&socket, SIGNAL(sslErrors(const QList<QSslError> &)));

    connect(&socket, SIGNAL(connected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(disconnected()), this, SLOT(exitLoop()));
    connect(&socket, SIGNAL(modeChanged(QSslSocket::SslMode)), this, SLOT(exitLoop()));
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
    QCOMPARE(socket.mode(), QSslSocket::UnencryptedMode);
    QVERIFY(!socket.isEncrypted());
    QCOMPARE(connectedSpy.count(), 1);
    QCOMPARE(hostFoundSpy.count(), 1);
    QCOMPARE(disconnectedSpy.count(), 0);

    // Enter encrypted mode
    socket.startClientEncryption();
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
    if (!QSslSocket::supportsSsl())
        return;

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
    socket.startClientEncryption();
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
    if (!QSslSocket::supportsSsl())
        return;
}

void tst_QSslSocket::addCaCertificates()
{
    if (!QSslSocket::supportsSsl())
        return;
}

void tst_QSslSocket::addCaCertificates2()
{
    if (!QSslSocket::supportsSsl())
        return;
}

void tst_QSslSocket::ciphers()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());
    socket.setCiphers(QList<QSslCipher>());
    QVERIFY(socket.ciphers().isEmpty());
    socket.setCiphers(socket.defaultCiphers());
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());
    socket.setCiphers(socket.defaultCiphers());
    QCOMPARE(socket.ciphers(), QSslSocket::supportedCiphers());

    // Task 164356
    socket.setCiphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
}

void tst_QSslSocket::connectToHostEncrypted()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;

    socket.addDefaultCaCertificates(QLatin1String("certs/fluke.ca.pem"));
    socket.connectToHostEncrypted("fluke.troll.no", 443);

    // This should pass unconditionally when using fluke's CA certificate.
    QVERIFY(socket.waitForEncrypted(10000));

    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected());

    QCOMPARE(socket.mode(), QSslSocket::SslClientMode);

    socket.connectToHost("fluke.troll.no", 13);

    QCOMPARE(socket.mode(), QSslSocket::UnencryptedMode);

    QVERIFY(socket.waitForDisconnected());
}

void tst_QSslSocket::sessionCipher()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    this->socket = &socket;
    connect(&socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(ignoreErrorSlot()));
    QVERIFY(socket.sessionCipher().isNull());
    socket.connectToHost("fluke.troll.no", 443 /* https */);
    QVERIFY(socket.waitForConnected(5000));
    QVERIFY(socket.sessionCipher().isNull());
    socket.startClientEncryption();
    QVERIFY(socket.waitForEncrypted(5000));
    QVERIFY(!socket.sessionCipher().isNull());
    QVERIFY(QSslSocket::supportedCiphers().contains(socket.sessionCipher()));
    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected());
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
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    QCOMPARE(socket.protocol(), QSsl::SslV3);
    {
        // Fluke allows SSLv3.
        socket.setProtocol(QSsl::SslV3);
        QCOMPARE(socket.protocol(), QSsl::SslV3);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        QCOMPARE(socket.protocol(), QSsl::SslV3);
        socket.abort();
        QCOMPARE(socket.protocol(), QSsl::SslV3);
        socket.connectToHost(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForConnected(), qPrintable(socket.errorString()));
        socket.startClientEncryption();
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        QCOMPARE(socket.protocol(), QSsl::SslV3);
        socket.abort();
    }
    {
        // Fluke allows TLSV1.
        socket.setProtocol(QSsl::TlsV1);
        QCOMPARE(socket.protocol(), QSsl::TlsV1);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        QCOMPARE(socket.protocol(), QSsl::TlsV1);
        socket.abort();
        QCOMPARE(socket.protocol(), QSsl::TlsV1);
        socket.connectToHost(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForConnected(), qPrintable(socket.errorString()));
        socket.startClientEncryption();
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        QCOMPARE(socket.protocol(), QSsl::TlsV1);
        socket.abort();
    }
    {
        // Fluke allows SSLV2.
        socket.setProtocol(QSsl::SslV2);
        QCOMPARE(socket.protocol(), QSsl::SslV2);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY(socket.waitForEncrypted());
        QCOMPARE(socket.protocol(), QSsl::SslV2);
        socket.abort();
        QCOMPARE(socket.protocol(), QSsl::SslV2);
        socket.connectToHost(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForConnected(), qPrintable(socket.errorString()));
        socket.startClientEncryption();
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        socket.abort();
    }
    {
        // Fluke allows SSLV3, so it allows AnyProtocol.
        socket.setProtocol(QSsl::AnyProtocol);
        QCOMPARE(socket.protocol(), QSsl::AnyProtocol);
        socket.connectToHostEncrypted(QLatin1String("fluke.troll.no"), 443);
        QVERIFY(socket.waitForEncrypted());
        QCOMPARE(socket.protocol(), QSsl::AnyProtocol);
        socket.abort();
        QCOMPARE(socket.protocol(), QSsl::AnyProtocol);
        socket.connectToHost(QLatin1String("fluke.troll.no"), 443);
        QVERIFY2(socket.waitForConnected(), qPrintable(socket.errorString()));
        socket.startClientEncryption();
        QVERIFY2(socket.waitForEncrypted(), qPrintable(socket.errorString()));
        QCOMPARE(socket.protocol(), QSsl::AnyProtocol);
        socket.abort();
    }
}

void tst_QSslSocket::setCaCertificates()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    QCOMPARE(socket.caCertificates(), QSslSocket::defaultCaCertificates());
    socket.setCaCertificates(QSslCertificate::fromPath("certs/fluke.ca.pem"));
    QCOMPARE(socket.caCertificates().size(), 1);
    socket.setCaCertificates(socket.defaultCaCertificates());
    QCOMPARE(socket.caCertificates(), QSslSocket::defaultCaCertificates());
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

class SslServer : public QTcpServer
{
    Q_OBJECT
public:
    SslServer() : socket(0) { }
    QSslSocket *socket;

protected:
    void incomingConnection(int socketDescriptor)
    {
        socket = new QSslSocket(this);
        connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));

        QFile file("certs/fluke.key");
        QVERIFY(file.open(QIODevice::ReadOnly));
        QSslKey key(file.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        QVERIFY(!key.isNull());
        socket->setPrivateKey(key);
        
        QList<QSslCertificate> localCert = QSslCertificate::fromPath("certs/fluke.cert");
        QVERIFY(!localCert.isEmpty());
        QVERIFY(localCert.first().handle());
        socket->setLocalCertificate(localCert.first());

        QVERIFY(socket->setSocketDescriptor(socketDescriptor, QAbstractSocket::ConnectedState));
        QVERIFY(!socket->peerAddress().isNull());
        QVERIFY(socket->peerPort() != 0);
        QVERIFY(!socket->localAddress().isNull());
        QVERIFY(socket->localPort() != 0);

        socket->startServerEncryption();
    }

protected slots:
    void ignoreErrorSlot()
    {
        socket->ignoreSslErrors();
    }
};

void tst_QSslSocket::setSocketDescriptor()
{
    if (!QSslSocket::supportsSsl())
        return;

    SslServer server;
    QVERIFY(server.listen());

    QEventLoop loop;
    QTimer::singleShot(5000, &loop, SLOT(quit()));

    QSslSocket *client = new QSslSocket;
    socket = client;
    connect(socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    connect(client, SIGNAL(encrypted()), &loop, SLOT(quit()));

    client->connectToHostEncrypted(QHostAddress(QHostAddress::LocalHost).toString(), server.serverPort());

    loop.exec();

    QCOMPARE(client->state(), QAbstractSocket::ConnectedState);
    QVERIFY(client->isEncrypted());
    QVERIFY(!client->peerAddress().isNull());
    QVERIFY(client->peerPort() != 0);
    QVERIFY(!client->localAddress().isNull());
    QVERIFY(client->localPort() != 0);
}

void tst_QSslSocket::waitForEncrypted()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    this->socket = &socket;

    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    socket.connectToHostEncrypted("fluke.troll.no", 993);

    QVERIFY(socket.waitForEncrypted(10000));
}

void tst_QSslSocket::waitForConnectedEncryptedReadyRead()
{
    if (!QSslSocket::supportsSsl())
        return;

    QSslSocket socket;
    this->socket = &socket;

    connect(&socket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(ignoreErrorSlot()));
    socket.connectToHostEncrypted("fluke.troll.no", 993);

    QVERIFY(socket.waitForConnected(10000));
    QVERIFY(socket.waitForEncrypted(10000));
    QVERIFY(socket.waitForReadyRead(10000));
    QVERIFY(!socket.peerCertificate().isNull());
    QVERIFY(!socket.peerCertificateChain().isEmpty());
}

void tst_QSslSocket::startClientEncryption()
{
}

void tst_QSslSocket::startServerEncryption()
{
}

void tst_QSslSocket::addDefaultCaCertificate()
{
    if (!QSslSocket::supportsSsl())
        return;

    // Reset the global CA chain
    QSslSocket::setDefaultCaCertificates(QSslSocket::systemCaCertificates());

    QList<QSslCertificate> flukeCerts = QSslCertificate::fromPath("certs/fluke.ca.pem");
    QCOMPARE(flukeCerts.size(), 1);
    QList<QSslCertificate> globalCerts = QSslSocket::defaultCaCertificates();
    QVERIFY(!globalCerts.contains(flukeCerts.first()));
    QSslSocket::addDefaultCaCertificate(flukeCerts.first());
    QCOMPARE(QSslSocket::defaultCaCertificates().size(), globalCerts.size() + 1);
    QVERIFY(QSslSocket::defaultCaCertificates().contains(flukeCerts.first()));

    // Restore the global CA chain
    QSslSocket::setDefaultCaCertificates(QSslSocket::systemCaCertificates());
}

void tst_QSslSocket::addDefaultCaCertificates()
{
}

void tst_QSslSocket::addDefaultCaCertificates2()
{
}

void tst_QSslSocket::defaultCaCertificates()
{
    if (!QSslSocket::supportsSsl())
        return;

    QList<QSslCertificate> certs = QSslSocket::defaultCaCertificates();
    QVERIFY(certs.size() > 1);
    QCOMPARE(certs, QSslSocket::systemCaCertificates());
}

void tst_QSslSocket::defaultCiphers()
{
}

void tst_QSslSocket::resetDefaultCiphers()
{
}

void tst_QSslSocket::setDefaultCaCertificates()
{
}

void tst_QSslSocket::setDefaultCiphers()
{
}

void tst_QSslSocket::supportedCiphers()
{
    if (!QSslSocket::supportsSsl())
        return;

    QList<QSslCipher> ciphers = QSslSocket::supportedCiphers();
    QVERIFY(ciphers.size() > 1);

    QSslSocket socket;
    QCOMPARE(socket.supportedCiphers(), ciphers);
    QCOMPARE(socket.defaultCiphers(), ciphers);
    QCOMPARE(socket.ciphers(), ciphers);
}

void tst_QSslSocket::systemCaCertificates()
{
    if (!QSslSocket::supportsSsl())
        return;

    QList<QSslCertificate> certs = QSslSocket::systemCaCertificates();
    QVERIFY(certs.size() > 1);
    QCOMPARE(certs, QSslSocket::defaultCaCertificates());
}

void tst_QSslSocket::wildcard()
{
    // Fluke runs an apache server listening on port 4443, serving the
    // wildcard fluke.*.troll.no.  The DNS entry for
    // fluke.wildcard.dev.troll.no, served by ares (root for dev.troll.no),
    // returns the CNAME fluke.troll.no for this domain. The web server
    // responds with the wildcard, and QSslSocket should accept that as a
    // valid connection.  This was broken in 4.3.0.
    QSslSocket socket;
    socket.connectToHostEncrypted("fluke.wildcard.dev.troll.no", 4443);

    QVERIFY2(socket.waitForEncrypted(), socket.peerCertificate().effectiveDate().toString().toLatin1());

    QSslCertificate certificate = socket.peerCertificate();
    QCOMPARE(certificate.subjectInfo(QSslCertificate::CommonName), QString("fluke.*.troll.no"));
    QCOMPARE(certificate.issuerInfo(QSslCertificate::CommonName), QString("fluke.troll.no"));

    socket.close();
}

#endif // QT_NO_OPENSSL

QTEST_MAIN(tst_QSslSocket)
#include "tst_qsslsocket.moc"
