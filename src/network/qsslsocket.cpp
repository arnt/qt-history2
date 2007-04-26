/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
**
** In addition, as a special exception, Trolltech gives permission to link
** the code of its release of Qt with the OpenSSL project's "OpenSSL" library
** (or modified versions of the "OpenSSL" library that use the same license
** as the original version), and distribute the linked executables.
**
** You must comply with the GNU General Public License version 2 in all
** respects for all of the code used other than the "OpenSSL" code.  If you
** modify this file, you may extend this exception to your version of the file,
** but you are not obligated to do so.  If you do not wish to do so, delete
** this exception statement from your version of this file.
**
****************************************************************************/

//#define QSSLSOCKET_DEBUG

/*!
    \class QSslSocket
    \brief The QSslSocket class provides an SSL encrypted socket for both
    clients and servers.
    \since 4.3

    \reentrant
    \ingroup io
    \module network

    QSslSocket establishes a secure, encrypted TCP connection you can
    use for transmitting encrypted data. It can operate in both client
    and server mode, and it supports modern SSL protocols, including
    SSLv3 and TLSv1. By default, QSslSocket uses SSLv3, but you can
    change the SSL protocol by calling setProtocol() as long as you do
    it before the handshake has started.

    SSL encryption operates on top of the existing TCP stream after
    the socket enters the ConnectedState. There are two simple ways to
    establish a secure connection using QSslSocket: With an immediate
    SSL handshake, or with a delayed SSL handshake occurring after the
    connection has been established in unencrypted mode.

    The most common way to use QSslSocket is to construct an object
    and start a secure connection by calling connectToHostEncrypted().
    This method starts an immediate SSL handshake once the connection
    has been established.

    \code
        QSslSocket *socket = new QSslSocket(this);
        connect(socket, SIGNAL(encrypted()), this, SLOT(ready()));

        socket->connectToHostEncrypted("imap.example.com", 993);
    \endcode

    As with a plain QTcpSocket, QSslSocket enters the HostLookupState,
    ConnectingState, and finally the ConnectedState, if the connection
    is successful. The hand shake then starts automatically, and if it
    succeeds, the encrypted() signal is emitted to indicate the socket
    has entered the encrypted state and is ready for use.

    Note that data can be written to the socket immediately after the
    return from connectToHostEncrypted() (i.e., before the encrypted()
    signal is emitted). The data is queued in QSslSocket until after
    the encrypted() signal is emitted.

    An example of using the delayed SSL handshake to secure an
    existing connection is the case where an SSL server secures an
    incoming connection. Suppose you create an SSL server class as a
    subclass of QTcpServer. You would reimplement
    QTcpServer::incomingConnection() with something like the example
    below, which first constructs an instance of QSslSocket and then
    calls setSocketDescriptor() to set the socket desxcriptor to the
    one passed in. Then the server initiates the SSL handshake by
    calling startServerEncryption().

    \code
        void SslServer::incomingConnection(int socketDescriptor)
        {
            QSslSocket *serverSocket = new QSslSocket;
            if (serverSocket->setSocketDescriptor(socketDescriptor)) {
                connect(serverSocket, SIGNAL(encrypted()), this, SLOT(ready()));
                serverSocket->startServerEncryption();
            } else {
                delete serverSocket;
            }
        }
    \endcode
    
    If an error occurs, QSslSocket emits signal sslErrors(). In that
    case, if no action is taken to ignore the error(s), the connection
    is dropped. To continue, despite the occurrance of an error, you
    can call ignoreSslErrors(), either from within this slot after the
    error occurs, or anytime after construction of the QSslSocket and
    before the connection is attempted. This will allow QSslSocket to
    ignore the errors it encounters when establishing the identity of
    the peer. Ignoring errors during an SSL handshake should be used
    with caution, since a fundamental characteristic of secure
    connections is that they should be established with a successful
    handshake.
    
    Once encrypted, you use QSslSocket as a regular QTcpSocket. When
    readyRead() is emitted, you can call read(), canReadLine() and
    readLine(), or getChar() to read decrypted data from QSslSocket's
    internal buffer, and you can call write() or putChar() to write
    data back to the peer. QSslSocket will automatically encrypt the
    written data for you, and emit bytesWritten() once the data has
    been written to the peer.

    As a convenience, QSslSocket supports QTcpSocket's blocking
    functions waitForConnected(), waitForReadyRead(),
    waitForBytesWritten(), and waitForDisconnected(). It also provides
    waitForEncrypted(), which will block the calling thread until an
    encrypted connection has been established.

    \code
        QSslSocket socket;
        socket.connectToHostEncrypted("http.example.com", 443);
        if (!socket.waitForEncrypted()) {
            qDebug() << socket.errorString();
            return false;
        }

        socket.write("GET / HTTP/1.0\r\n\r\n");
        while (socket.waitForReadyRead())
            qDebug() << socket.readAll().data();
    \endcode

    QSslSocket provides an extensive, easy-to-use API for handling SSL
    certificates, ciphers, and for managing errors. You can customize
    the socket's cryptographic cipher suite and CA database by calling
    setCiphers() or setCaCertificates(). For more information about
    ciphers, refer to QSslCipher's documentation. You can read about
    SSL certificates in the class documentation for QSslCertificate.
    
    This product includes software developed by the OpenSSL Project
    for use in the OpenSSL Toolkit (http://www.openssl.org/).

    \sa QSslCertificate, QSslCipher, QSslError
*/

/*!
    \enum QSslSocket::SslMode

    Describes the different connection modes a QSslSocket can be in.

    \value UnencryptedMode The socket is unencrypted, and behaves identically
    to QTcpSocket.

    \value SslClientMode The socket is a client-side SSL socket, and is either
    encrypted, or in the handshake phase (see QSslSocket::isEncrypted()).

    \value SslServerMode The socket is a client-side SSL socket, and is either
    encrypted, or in the handshake phase (see QSslSocket::isEncrypted()).
*/

/*!
    \enum QSslSocket::Protocol

    Describes the protocol to use for the connection.
    
    \value SslV3 SSLv3
    \value SslV2 SSLv2
    \value TlsV1 TLSv1
    \value AnyProtocol The socket understands SSLv2, SSLv3 and TLSv1.
*/

/*!
    \fn QSslSocket::encrypted()

    This signal is emitted when QSslSocket enters encrypted mode. After this
    signal has been emitted, QSslSocket::isEncrypted() will return true, and
    all further transmissions on the socket will be encrypted.

    \sa QSslSocket::connectToHostEncrypted(), QSslSocket::isEncrypted()
*/

/*!
    \fn QSslSocket::modeChanged(QSslSocket::SslMode mode)

    This signal is emitted when QSslSocket changes from \l
    QSslSocket::UnencryptedMode to either \l QSslSocket::SslClientMode or \l
    QSslSocket::SslServerMode. \a mode is the new mode.

    \sa QSslSocket::mode()
*/

/*!
    \fn QSslSocket::sslErrors(const QList<QSslError> &errors)

    QSslSocket emits this signal during the SSL handshake to indicate that an
    error has occurred while establishing the identity of the peer. The error
    is usually an indication that QSslSocket is unable to securely identify
    the peer. Unless any action is taken, the connection will be dropped after
    this signal has been emitted.

    If you want to continue connecting despite the errors that have occurred,
    you must call QSslSocket::ignoreErrors() from inside a slot connected to
    this signal. If you need to access the error list at a later point, you
    can call sslErrors() (without arguments).

    \a errors contains one or more errors that prevent QSslSocket from
    verifying the identity of the peer.
    
    Note: You cannot use Qt::QueuedConnection when connecting to this signal,
    or calling QSslSocket::ignoreErrors() will have no effect.
*/

#include "qsslcipher.h"
#include "qsslsocket.h"
#include "qsslsocket_openssl_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmutex.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qhostinfo.h>

class QSslSocketGlobalData
{
public:
    QMutex mutex;
    QList<QSslCipher> ciphers;
    QList<QSslCipher> supportedCiphers;
    QList<QSslCertificate> caCertificates;
};
Q_GLOBAL_STATIC(QSslSocketGlobalData, globalData)

/*!
    Constructs a QSslSocket object. \a parent is passed to QObject's
    constructor.
*/
QSslSocket::QSslSocket(QObject *parent)
    : QTcpSocket(*new QSslSocketBackendPrivate, parent)
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::QSslSocket(" << parent << "), this =" << (void *)this;
#endif
    d->q_ptr = this;
    d->init();
    setCiphers(defaultCiphers());
}

/*!
    Destroys the QSslSocket.
*/
QSslSocket::~QSslSocket()
{
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::~QSslSocket(), this =" << (void *)this;
#endif
}

/*!
    Starts an encrypted connection to the device \a hostName on \a
    port, using \a mode as the \l OpenMode. This is equivalent to
    calling connectToHost() to establish the connection, followed by a
    call to startClientEncryption().

    QSslSocket first enters the HostLookupState. Then, after entering
    either the event loop or one of the waitFor...() functions, it
    enters the ConnectingState, emits connected(), and then initiates
    the SSL client handshake. At each state change, QSslSocket emits
    signal stateChanged().

    After initiating the SSL client handshake, if the identity of the
    peer can't be established, signal sslErrors() is emitted. If you
    want to ignore the errors and continue connecting, you must call
    ignoreSslErrors(), either from inside a slot function connected to
    the sslErrors() signal, or prior to entering encrypted mode. If
    ignoreSslErrors is not called, the connection is dropped, signal
    disconnected() is emitted, and QSslSocket returns to the
    UnconnectedState.

    If the SSL handshake is successful, QSslSocket emits encrypted().

    \code
        QSslSocket socket;
        connect(&socket, SIGNAL(encrypted()), receiver, SLOT(socketEncrypted()));

        socket.connectToHostEncrypted("imap", 993);
        socket->write("1 CAPABILITY\r\n");
    \endcode

    Note, in the example above, text can be written to the socket
    immediately after requesting the encrypted connection, before the
    encrypted() signal has been emitted. In such cases, the text is
    queued in the object and written to the socket \e after the
    connection is established and the encrypted() signal is emitted.

    The default for \a mode is \l ReadWrite.

    If you want to create a QSslSocket on the server side of a connection, you
    should instead call startServerEncryption() upon receiving the incoming
    connection through QTcpServer.

    \sa connectToHost(), startClientEncryption(), waitForConnected(), waitForEncrypted()
*/
void QSslSocket::connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode)
{
    Q_D(QSslSocket);
    if (d->state == ConnectedState || d->state == ConnectingState) {
        qWarning("QSslSocket::connectToHostEncrypted() called when already connecting/connected");
        return;
    }

    connectToHost(hostName, port, mode);
    d->autoStartHandshake = true;
}

/*!
    Initializes QSslSocket with the native socket descriptor \a
    socketDescriptor. Returns true if \a socketDescriptor is accepted
    as a valid socket descriptor; otherwise returns false.
    The socket is opened in the mode specified by \a openMode, and
    enters the socket state specified by \a state.

    \bold{Note:} It is not possible to initialize two sockets with the same
    native socket descriptor.

    \sa socketDescriptor()
*/
bool QSslSocket::setSocketDescriptor(int socketDescriptor, SocketState state, OpenMode openMode)
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::setSocketDescriptor(" << socketDescriptor << ","
             << state << "," << openMode << ")";
#endif
    if (!d->plainSocket)
        d->createPlainSocket(openMode);
    bool retVal = d->plainSocket->setSocketDescriptor(socketDescriptor, state, openMode);
    d->cachedSocketDescriptor = d->plainSocket->socketDescriptor();
    setSocketError(d->plainSocket->error());
    setSocketState(state);
    setOpenMode(openMode);
    return retVal;
}

/*!
    Returns the current mode for the socket; either UnencryptedMode, where
    QSslSocket behaves identially to QTcpSocket, or one of SslClientMode or
    SslServerMode, where the client is either negotiating or in encrypted
    mode.

    When the mode changes, QSslSocket emits modeChanged()

    \sa SslMode
*/
QSslSocket::SslMode QSslSocket::mode() const
{
    Q_D(const QSslSocket);
    return d->mode;
}

/*!
    Returns true if the socket is encrypted; otherwise, false is returned.

    An encrypted socket encrypts all data that is written by calling write()
    or putChar() before the data is written to the network, and descrypts all
    incoming data as the data is received from the network, before you call
    read(), readLine() or getChar().

    QSslSocket emits encrypted() when it enters encrypted mode.

    You can call sessionCipher() to find which cryptographic cipher is used to
    encrypt and decrypt your data.

    \sa mode()
*/
bool QSslSocket::isEncrypted() const
{
    Q_D(const QSslSocket);
    return d->connectionEncrypted;
}

/*!
    Returns the socket's SSL protocol. By default, \l SslV3 is used.

    \value setProtocol()
*/
QSslSocket::Protocol QSslSocket::protocol() const
{
    Q_D(const QSslSocket);
    return d->protocol;
}

/*!
    Sets the socket's SSL protocol to \a protocol. This will affect the next
    initiated handshake; calling this function on an already-encrypted socket
    will not affect the socket's protocol.
*/
void QSslSocket::setProtocol(Protocol protocol)
{
    Q_D(QSslSocket);
    d->protocol = protocol;
}

/*!
    \reimp

    Returns the number of decrypted bytes that are immediately available for
    reading.
*/
qint64 QSslSocket::bytesAvailable() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::bytesAvailable() + (d->plainSocket ? d->plainSocket->bytesAvailable() : 0);
    return QIODevice::bytesAvailable() + d->readBuffer.size();
}

/*!
    \reimp

    Returns the number of uneccrypted bytes that are waiting to be encrypted
    and written to the network.
*/
qint64 QSslSocket::bytesToWrite() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return d->plainSocket ? d->plainSocket->bytesToWrite() : 0;
    return d->writeBuffer.size();
}

/*!
    \reimp

    Returns true if you can read one while line (terminated by a single ASCII
    '\n' character) of decrypted characters; otherwise, false is returned.
*/
bool QSslSocket::canReadLine() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::canReadLine() || (d->plainSocket && d->plainSocket->canReadLine());
    return QIODevice::canReadLine() || (!d->readBuffer.isEmpty() && d->readBuffer.canReadLine());
}

/*!
    \reimp
*/
void QSslSocket::close()
{
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::close()";
#endif
    QTcpSocket::close();
}

/*!
    \reimp
*/
bool QSslSocket::atEnd() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::atEnd() && (!d->plainSocket || d->plainSocket->atEnd());
    return QIODevice::atEnd() && d->readBuffer.isEmpty();
}

/*!
    This function writes as much as possible from the internal write buffer to
    the underlying network socket, without blocking. If any data was written,
    this function returns true; otherwise false is returned.

    Call this function if you need QSslSocket to start sending buffered data
    immediately. The number of bytes successfully written depends on the
    operating system. In most cases, you do not need to call this function,
    because QAbstractSocket will start sending data automatically once control
    goes back to the event loop. In the absence of an event loop, call
    waitForBytesWritten() instead.

    \sa write(), waitForBytesWritten()
*/
// Note! docs copied from QAbstractSocket::flush()
bool QSslSocket::flush()
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::flush()";
#endif
    return d->plainSocket ? d->plainSocket->flush() : false;
}

/*!
    Aborts the current connection and resets the socket. Unlike
    disconnectFromHost(), this function immediately closes the socket,
    clearing any pending data in the write buffer.

    \sa disconnectFromHost(), close()
*/
// Note! docs copied from QAbstractSocket::aborts()
void QSslSocket::abort()
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::abort()";
#endif
    if (d->plainSocket)
        d->plainSocket->abort();
}

/*!
    Sets the socket's local certificate to \a certificate. The local
    certificate is necessary if you need to confirm your identity to the
    peer. It is used together with the private key; if you set the local
    certificate, you must also set the private key.

    The local certificate and private key are always necessary for server
    sockets, but are also rarely used by client sockets if the server requires
    the client to authenticate.

    \sa localCertificate(), setPrivateKey()
*/
void QSslSocket::setLocalCertificate(const QSslCertificate &certificate)
{
    Q_D(QSslSocket);
    d->localCertificate = certificate;
}

/*!
    \overload

    Sets the local certificate to \a path, using \a format encoding.
*/
void QSslSocket::setLocalCertificate(const QString &path, QSsl::EncodingFormat format)
{
    Q_D(QSslSocket);
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        d->localCertificate = QSslCertificate(file.readAll(), format);
}

/*!
    Returns the socket's local certificate, or an empty certificate if no
    local certificate has been assigned.

    \sa setLocalCertificate(), privateKey()
*/
QSslCertificate QSslSocket::localCertificate() const
{
    Q_D(const QSslSocket);
    return d->localCertificate;
}

/*!
    Returns the peer's certificate (i.e., the immediate certificate of the
    host you are connected to), or a null certificate if either the peer
    hasn't provided any certificate (common for server sockets).

    The peer certificate is provided for connection diagnostic purposes, and
    it's commonly used for displaying to the user. It contains information
    about the peer, including its host name, the certificate issuer, and the
    peer's public key.

    The peer certificate is set during the handshake phase, so it's safe to
    check this certificate from inside a slot connected to the sslErrors() or
    encrypted() signals.

    If you also want to check the rest of the peer's chain of certificates,
    you can call peerCertificateChain().

    \sa peerCertificateChain()
*/
QSslCertificate QSslSocket::peerCertificate() const
{
    Q_D(const QSslSocket);
    return d->peerCertificate;
}

/*!
    Returns the peer's chain of certificates, or an empty list of certificates
    if the peer either hasn't provided any certificates.

    The peer certificate chain is provided for connection diagnostic purposes,
    and it's commonly used for displaying to the user. It contains information
    about the peer, including its host name, the certificate issuer and its
    chain of authorities, and the peer's and issuer's public keys.

    The peer certificates are set during the handshake phase, so it's safe to
    check the certificate chain from inside a slot connected to the
    sslErrors() or encrypted() signals.

    If all you want is to check the peer's own certificate, you can call
    peerCertificateChain() instead.

    \sa peerCertificate()
*/
QList<QSslCertificate> QSslSocket::peerCertificateChain() const
{
    Q_D(const QSslSocket);
    return d->peerCertificateChain;
}

/*!
    Returns the socket's current cryptographic cipher, or a null cipher if the
    connection isn't encrypted. You can call this function to find information
    about the cipher that is used to encrypt and decrypt all data transmitted
    through this socket.

    QSslSocket also provides functions for selecting which ciphers should be
    used for encrypting data.

    \sa ciphers(), setCiphers(), setDefaultCiphers(), defaultCiphers(),
    supportedCiphers()
*/
QSslCipher QSslSocket::sessionCipher() const
{
    Q_D(const QSslSocket);
    return d->sessionCipher();
}

/*!
    Sets the socket's private key to \a key. The private key and local
    certificate are used by clients or servers that need to prove their
    identity to the peer. This key and the local certificate are necessary for
    all SSL server sockets, but are rarely also used by clients that need to
    authenticate against a server.

    \sa privateKey(), setLocalCertificate()
*/
void QSslSocket::setPrivateKey(const QSslKey &key)
{
    Q_D(QSslSocket);
    d->privateKey = key;
}

/*!
    \overload

    Loads the private key at \a fileName, using \a algorithm and \a format. If
    the key is encrypted, \a passPhrase will be passed to decrypt the key.

    \sa privateKey(), setLocalCertificate()
*/
void QSslSocket::setPrivateKey(const QString &fileName, QSsl::Algorithm algorithm,
                               QSsl::EncodingFormat format,
                               const QByteArray &passPhrase)
{
    Q_D(QSslSocket);
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        d->privateKey = QSslKey(file.readAll(), algorithm, format,
                                QSsl::PrivateKey, passPhrase);
    }
}

/*!
    Returns this socket's private key.

    \sa setPrivateKey(), localCertificate()
*/
QSslKey QSslSocket::privateKey() const
{
    Q_D(const QSslSocket);
    return d->privateKey;
}

/*!
    Returns this socket's current cryptographic cipher suite. This list is
    used during the socket's handshake phase when negotiating ciphers with the
    peer. The list is returned in descending preferred order (i.e., the first
    cipher in the list is the most preferred cipher).

    By default, the socket will use supportedCiphers(), a predefined set of
    ciphers that works for most common cases. This predefined set is defined
    by the current SSL libraries, and may vary from system to system. If you
    change the ciphers for this socket, you can later call setCiphers() at
    any time to revert to using the default set.

    \sa setCiphers(), defaultCiphers(), supportedCiphers()
*/
QList<QSslCipher> QSslSocket::ciphers() const
{
    Q_D(const QSslSocket);
    return d->ciphers;
}

/*!
    Sets the cryptographic cipher suite for this socket to \a ciphers.

    \sa ciphers(), setDefaultCiphers(), supportedCiphers()
*/
void QSslSocket::setCiphers(const QList<QSslCipher> &ciphers)
{
    Q_D(QSslSocket);
    d->ciphers = ciphers;
}

/*!
    Sets the cryptographic cipher suite for this socket to \a ciphers, which
    is a colon-separated list of cipher names. The ciphers are listed in order
    of preference, starting with the most preferred cipher. For example:

    \code
        QSslSocket socket;
        socket.setCiphers("!ADH:RC4+RSA:HIGH:MEDIUM:LOW:EXP:+SSLv2:+EXP");
    \endcode

    \sa ciphers(), setDefaultCiphers(), supportedCiphers()
*/
void QSslSocket::setCiphers(const QString &ciphers)
{
    Q_D(QSslSocket);
    d->ciphers.clear();
    foreach (QString cipherName, ciphers.split(QLatin1String(":"), QString::SkipEmptyParts)) {
        for (int i = 0; i < 3; ++i) {
            // ### Crude
            QSslCipher cipher(cipherName, QSslCipher::Protocol(i));
            if (!cipher.isNull())
                d->ciphers << cipher;
        }
    }
}

/*!
    Sets the default cryptographic cipher suite for all sockets in this
    application to \a ciphers.

    \sa setCiphers(), defaultCiphers(), supportedCiphers()
*/
void QSslSocket::setDefaultCiphers(const QList<QSslCipher> &ciphers)
{
    QSslSocketPrivate::setDefaultCiphers(ciphers);
}

/*!
    Returns the default cryptographic cipher suite for all sockets in this
    application. This list is used during the socket's handshake phase when
    negotiating ciphers with the peer. The list is returned in descending
    preferred order (i.e., the first cipher in the list is the most preferred
    cipher).

    By default, the system will use a predefined set of ciphers that works for
    most common cases. This set is defined by the current SSL libraries, and
    may vary from system to system.

    \sa supportedCiphers()
*/
QList<QSslCipher> QSslSocket::defaultCiphers()
{
    return QSslSocketPrivate::defaultCiphers();
}

/*!
    Returns the list of cryptographic ciphers supported by this system.

    This list is determined by the current SSL libraries, and may vary from
    system to system.

    \sa defaultCiphers(), ciphers(), setCiphers()
*/
QList<QSslCipher> QSslSocket::supportedCiphers()
{
    return QSslSocketPrivate::supportedCiphers();
}

/*!
    Adds all CA certificates in \a path using \a format encoding, which may be
    a file, or a directory with \a syntax formatted wildcards. Returns true on
    success; otherwise returns false.

    For more fine grained control, you can call addCaCertificate() instead.

    \sa addCaCertificate()
*/
bool QSslSocket::addCaCertificates(const QString &path, QSsl::EncodingFormat format,
                                   QRegExp::PatternSyntax syntax)
{
    Q_D(QSslSocket);
    QList<QSslCertificate> certs = QSslCertificate::fromPath(path, format, syntax);
    if (certs.isEmpty())
        return false;

    d->localCaCertificates += certs;
    return true;
}

/*!
    Adds \a certificate to this socket's CA certificate database.

    \sa caCertificates(), addDefaultCaCertificate()
*/
void QSslSocket::addCaCertificate(const QSslCertificate &certificate)
{
    Q_D(QSslSocket);
    d->localCaCertificates += certificate;
}

/*!
    Adds \a certificates to this socket's CA certificate database.

    \sa caCertificates(), addDefaultCaCertificate()
*/
void QSslSocket::addCaCertificates(const QList<QSslCertificate> &certificates)
{
    Q_D(QSslSocket);
    d->localCaCertificates += certificates;
}

/*!
    Sets \a certificates to be this socket's CA certificate database. Any
    global CA certificates are ignored.

    You can later call setCaCertificates() to restore the global CA
    certificate defaults.

    \sa caCertificates(), addDefaultCaCertificate()
*/
void QSslSocket::setCaCertificates(const QList<QSslCertificate> &certificates)
{
    Q_D(QSslSocket);
    d->useLocalCaCertificatesOnly = true;
    d->localCaCertificates = certificates;
}

/*!
    Returns this socket's CA certificate database.

    \sa addCaCertificates(), defaultCaCertificates()
*/
QList<QSslCertificate> QSslSocket::caCertificates() const
{
    Q_D(const QSslSocket);
    if (d->useLocalCaCertificatesOnly)
        return d->localCaCertificates;
    return d->defaultCaCertificates() + d->localCaCertificates;
}

/*!
    Adds all CA certificates in \a path, using \a encoding, to the global CA
    certificate database.  \a path can be a file, or a directory with \a
    syntax formatted wildcards. Returns true on success; otherwise returns
    false.

    \sa defaultCaCertificates(), addCaCertificates(), addDefaultCaCertificate()
*/
bool QSslSocket::addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat encoding,
                                          QRegExp::PatternSyntax syntax)
{
    return QSslSocketPrivate::addDefaultCaCertificates(path, encoding, syntax);
}

/*!
    Adds \a certificate to the global CA certificate database.

    \sa defaultCaCertificates(), addCaCertificates()
*/
void QSslSocket::addDefaultCaCertificate(const QSslCertificate &certificate)
{
    QSslSocketPrivate::addDefaultCaCertificate(certificate);
}

/*!
    Adds \a certificates to the global CA certificate database.

    \sa defaultCaCertificates(), addCaCertificates()
*/
void QSslSocket::addDefaultCaCertificates(const QList<QSslCertificate> &certificates)
{
    QSslSocketPrivate::addDefaultCaCertificates(certificates);
}

/*!
    Sets \a certificates to be QSslSocket's global CA certificate database.

    \sa addDefaultCaCertificate()
*/
void QSslSocket::setDefaultCaCertificates(const QList<QSslCertificate> &certificates)
{
    // ### Document exactly hos this works.
    // ### We might need a setter, and perhaps a ban'er.
    QSslSocketPrivate::setDefaultCaCertificates(certificates);
}

/*!
    Returns the global CA certificate database.

    \sa caCertificates()
*/
QList<QSslCertificate> QSslSocket::defaultCaCertificates()
{
    return QSslSocketPrivate::defaultCaCertificates();
}

/*!
    Returns the default CA certificate database for the system.

    \sa caCertificates()
*/
QList<QSslCertificate> QSslSocket::systemCaCertificates()
{
    QSslSocketPrivate::ensureInitialized();
    return QSslSocketPrivate::systemCaCertificates();
}

/*!
    Waits until the socket is connected, up to \a msecs
    milliseconds. If the connection has been established, this
    function returns true; otherwise it returns false.

    \sa QAbstractSocket::waitForConnected()
*/
bool QSslSocket::waitForConnected(int msecs)
{
    Q_D(QSslSocket);
    if (!d->plainSocket)
        return false;
    bool retVal = d->plainSocket->waitForConnected(msecs);
    if (!retVal) {
        setSocketState(d->plainSocket->state());
        setSocketError(d->plainSocket->error());
        setErrorString(d->plainSocket->errorString());
    }
    return retVal;
}

/*!
    Waits until the socket has completed the SSL handshake and has emitted
    encrypted(), up to \a msecs milliseconds. If encrypted() has been emitted,
    this function returns true; otherwise (e.g., the socket is disconnected,
    or the SSL handshake fails), false is returned.

    The following example waits up to one second for the socket to be
    encrypted:

    \code
        socket->connectToHostEncrypted("imap", 993);
        if (socket->waitForEncrypted(1000))
            qDebug("Encrypted!");
    \endcode

    If msecs is -1, this function will not time out.

    \sa startClientEncryption(), startServerEncryption(), encrypted(), isEncrypted()
*/
bool QSslSocket::waitForEncrypted(int msecs)
{
    Q_D(QSslSocket);
    if (!d->plainSocket || d->connectionEncrypted)
        return false;
    if (d->mode == UnencryptedMode && !d->autoStartHandshake)
        return false;

    QTime stopWatch;
    stopWatch.start();

    if (d->plainSocket->state() != QAbstractSocket::ConnectedState) {
        // Wait until we've entered connected state.
        if (!d->plainSocket->waitForConnected(msecs))
            return false;
    }

    while (!d->connectionEncrypted) {
        // Start the handshake, if this hasn't been started yet.
        if (d->mode == UnencryptedMode)
            startClientEncryption();
        // Loop, waiting until the connection has been encrypted or an error
        // occurs.
        if (!d->plainSocket->waitForReadyRead(qBound(0, msecs - stopWatch.elapsed(), msecs)))
            return false;
    }
    return d->connectionEncrypted;
}

/*!
    \reimp
*/
bool QSslSocket::waitForReadyRead(int msecs)
{
    Q_D(QSslSocket);
    if (!d->plainSocket)
        return false;
    if (d->mode == UnencryptedMode && !d->autoStartHandshake)
        return d->plainSocket->waitForReadyRead(msecs);

    int oldReadBufferSize = d->readBuffer.size();
    
    QTime stopWatch;
    stopWatch.start();

    if (!d->connectionEncrypted) {
        // Wait until we've entered encrypted mode, or until a failure occurs.
        if (!waitForEncrypted(msecs))
            return false;
    }

    while (d->plainSocket->waitForReadyRead(qBound(0, msecs - stopWatch.elapsed(), msecs))) {
        if (d->readBuffer.size() != oldReadBufferSize) {
            // If the read buffer has grown, readyRead() must have been emitted.
            return true;
        }
    }
    return false;
}

/*!
    \reimp
*/
bool QSslSocket::waitForBytesWritten(int msecs)
{
    Q_D(QSslSocket);
    if (!d->plainSocket)
        return false;
    if (d->mode == UnencryptedMode)
        return d->plainSocket->waitForBytesWritten(msecs);

    QTime stopWatch;
    stopWatch.start();

    if (!d->connectionEncrypted) {
        // Wait until we've entered encrypted mode, or until a failure occurs.
        if (!waitForEncrypted(msecs))
            return false;
    }

    return d->plainSocket->waitForBytesWritten(qBound(0, msecs - stopWatch.elapsed(), msecs));
}

/*!
    Waits until the socket has disconnected, up to \a msecs
    milliseconds. If the connection has been disconnected, this
    function returns true; otherwise it returns false.

    \sa QAbstractSocket::waitForDisconnected()
*/
bool QSslSocket::waitForDisconnected(int msecs)
{
    Q_D(QSslSocket);

    // require calling connectToHost() before waitForDisconnected()
    if (state() == UnconnectedState) {
        qWarning("QSslSocket::waitForDisconnected() is not allowed in UnconnectedState");
        return false;
    }

    if (!d->plainSocket)
        return false;
    bool retVal = d->plainSocket->waitForDisconnected(msecs);
    if (!retVal) {
        setSocketState(d->plainSocket->state());
        setSocketError(d->plainSocket->error());
        setErrorString(d->plainSocket->errorString());
    }
    return retVal;
}

/*!
    Returns a list of the last SSL errors that occurred. This is the same list
    as QSslSocket passes via the sslErrors() signal. If the connection has
    been encrypted with no errors, this function will return an empty list.

    \sa connectToHostEncrypted()
*/
QList<QSslError> QSslSocket::sslErrors() const
{
    Q_D(const QSslSocket);
    return d->sslErrors;
}

/*!
    Returns true if this platform supports SSL; otherwise, returns false.

    If the platform doesn't support SSL, the socket will fail in the
    connection phase.
*/
bool QSslSocket::supportsSsl()
{
    return QSslSocketPrivate::ensureInitialized();
}

/*!
    Starts a delayed SSL handshake for a client connection. This function must
    be called when the socket is in \l UnencryptedMode, and \l ConnectedState;
    otherwise, it has no effect.

    Clients that implement STARTTLS functionality often make use of delayed
    SSL handshakes; most other clients can avoid calling this function
    directly by using connectToHostEncrypted() instead.

    \sa connectToHostEncrypted(), startServerEncryption()
*/
void QSslSocket::startClientEncryption()
{
    Q_D(QSslSocket);
    if (d->mode != UnencryptedMode) {
        qWarning("QSslSocket::startClientEncryption: cannot start handshake on non-plain connection");
        return;
    }
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::startClientEncryption()";
#endif
    d->mode = SslClientMode;
    emit modeChanged(d->mode);
    d->startClientEncryption();
}

/*!
    Starts a delayed SSL handshake for a server connection. This function must
    be called when the socket is in \l UnencryptedMode, and \l ConnectedState;
    otherwise, it has no effect.

    For server sockets, calling this function is the only way to initiate the
    SSL handshake. Most servers will call this function immediately upon
    receiving a connection, or as a result of having received a
    protocol-specific command to enter SSL mode (e.g, the server may respond
    to receiving the string "STARTTLS\r\n" by calling this function).

    The most common way to implement SSL servers is to create a subclass of
    QTcpServer, and reimplement QTcpServer::incomingConnection(). The provided
    socket descriptor is then passed to QSslSocket::setSocketDescriptor().
    
    \sa connectToHostEncrypted(), startClientEncryption()
*/
void QSslSocket::startServerEncryption()
{
    Q_D(QSslSocket);
    if (d->mode != UnencryptedMode) {
        qWarning("QSslSocket::startClientEncryption: cannot start handshake on non-plain connection");
        return;
    }
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::startServerEncryption()";
#endif
    d->mode = SslServerMode;
    emit modeChanged(d->mode);
    d->startServerEncryption();
}

/*!
    This slot tells QSslSocket to ignore errors during QSslSocket's
    handshake phase and continue connecting. If an error occurs during
    the handshake phase, but you want to continue with the connection
    anyway, then you must call this slot, either from a slot connected
    to sslErrors(), or before the attempt to enter encrypted mode. If
    you don't call this slot, either in response to errors or before
    connecting, the connection will be dropped after the sslErrors()
    signal has been emitted.

    If there are no errors during the SSL handshake phase (i.e., the
    identity of the peer is established with no problems), QSslSocket
    will not emit the sslErrors() signal, and it is unnecessary to
    call this function.

    Ignoring errors during an SSL handshake should be used with
    caution, since a fundamental characteristic of secure connections
    is that they should be established with a successful handshake.

    \sa sslErrors()
*/
void QSslSocket::ignoreSslErrors()
{
    Q_D(QSslSocket);
    d->ignoreSslErrors = true;
}

/*!
    \internal
*/
void QSslSocket::connectToHostImplementation(const QString &hostName, quint16 port,
                                             OpenMode openMode)
{
    Q_D(QSslSocket);
    d->init();
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::connectToHostImplementation("
             << hostName << "," << port << "," << openMode << ")";
#endif
    if (!d->plainSocket) {
#ifdef QSSLSOCKET_DEBUG
        qDebug() << "\tcreating internal plain socket";
#endif
        d->createPlainSocket(openMode);
#ifndef QT_NO_NETWORKPROXY
        d->plainSocket->setProxy(proxy());
#endif
    }
    setOpenMode(openMode);
    d->plainSocket->connectToHost(hostName, port, openMode);
    d->cachedSocketDescriptor = d->plainSocket->socketDescriptor();
}

/*!
    \internal
*/
void QSslSocket::disconnectFromHostImplementation()
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::disconnectFromHostImplementation()";
#endif
    if (!d->plainSocket)
        return;
    if (d->mode == UnencryptedMode) {
        d->plainSocket->disconnectFromHost();
    } else {
        d->disconnectFromHost();
    }
}

/*!
    \reimp
*/
qint64 QSslSocket::readData(char *data, qint64 maxlen)
{
    Q_D(QSslSocket);
    qint64 readBytes = 0;

    if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
        readBytes = d->plainSocket->read(data, maxlen);
    } else {
        do {
            const char *readPtr = d->readBuffer.readPointer();
            int bytesToRead = qMin<int>(maxlen, d->readBuffer.nextDataBlockSize());
            ::memcpy(data + readBytes, readPtr, bytesToRead);
            readBytes += bytesToRead;
            d->readBuffer.free(bytesToRead);
        } while (!d->readBuffer.isEmpty());
    }
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::readData(" << (void *)data << "," << maxlen << ") ==" << readBytes;
#endif
    return readBytes;
}

/*!
    \reimp
*/
qint64 QSslSocket::writeData(const char *data, qint64 len)
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::writeData(" << (void *)data << "," << len << ")";
#endif
    if (d->mode == UnencryptedMode && !d->autoStartHandshake)
        return d->plainSocket->write(data, len);

    d->writeBuffer.write(data, len);

    if (d->connectionEncrypted)
        d->transmit();

    return len;
}

/*!
    \internal
*/
QSslSocketPrivate::QSslSocketPrivate()
    : plainSocket(0)
{
}

/*!
    \internal
*/
QSslSocketPrivate::~QSslSocketPrivate()
{
}

/*!
    \internal
*/
void QSslSocketPrivate::init()
{
    mode = QSslSocket::UnencryptedMode;
    autoStartHandshake = false;
    connectionEncrypted = false;
    ignoreSslErrors = false;
    protocol = QSslSocket::SslV3;
    useLocalCaCertificatesOnly = false;

    readBuffer.clear();
    writeBuffer.clear();
    peerCertificate.clear();
}

/*!
    \internal
*/
QList<QSslCipher> QSslSocketPrivate::defaultCiphers()
{
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->ciphers;
}

/*!
    \internal
*/
QList<QSslCipher> QSslSocketPrivate::supportedCiphers()
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->supportedCiphers;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultCiphers(const QList<QSslCipher> &ciphers)
{
    QMutexLocker locker(&globalData()->mutex);
    globalData()->ciphers = ciphers;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultSupportedCiphers(const QList<QSslCipher> &ciphers)
{
    QMutexLocker locker(&globalData()->mutex);
    globalData()->supportedCiphers = ciphers;
}

/*!
    \internal
*/
QList<QSslCertificate> QSslSocketPrivate::defaultCaCertificates()
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->caCertificates;
}

/*!
    \internal
*/
void QSslSocketPrivate::setDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates = certs;
}

/*!
    \internal
*/
bool QSslSocketPrivate::addDefaultCaCertificates(const QString &path, QSsl::EncodingFormat format,
                                                 QRegExp::PatternSyntax syntax)
{
    QSslSocketPrivate::ensureInitialized();
    QList<QSslCertificate> certs = QSslCertificate::fromPath(path, format, syntax);
    if (certs.isEmpty())
        return false;

    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += certs;
    return true;
}

/*!
    \internal
*/
void QSslSocketPrivate::addDefaultCaCertificate(const QSslCertificate &cert)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += cert;
}

/*!
    \internal
*/
void QSslSocketPrivate::addDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += certs;
}

/*!
    \internal
*/
void QSslSocketPrivate::createPlainSocket(QIODevice::OpenMode openMode)
{
    Q_Q(QSslSocket);
    q->setOpenMode(openMode); // <- from QIODevice
    q->setSocketState(QAbstractSocket::UnconnectedState);
    q->setSocketError(QAbstractSocket::UnknownSocketError);
    q->setLocalPort(0);
    q->setLocalAddress(QHostAddress());
    q->setPeerPort(0);
    q->setPeerAddress(QHostAddress());
    q->setPeerName(QString());

    plainSocket = new QTcpSocket(q);
    q->connect(plainSocket, SIGNAL(connected()),
               q, SLOT(_q_connectedSlot()));
    q->connect(plainSocket, SIGNAL(hostFound()),
               q, SLOT(_q_hostFoundSlot()));
    q->connect(plainSocket, SIGNAL(disconnected()),
               q, SLOT(_q_disconnectedSlot()));
    q->connect(plainSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
               q, SLOT(_q_stateChangedSlot(QAbstractSocket::SocketState)));
    q->connect(plainSocket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(_q_errorSlot(QAbstractSocket::SocketError)));
    q->connect(plainSocket, SIGNAL(readyRead()),
               q, SLOT(_q_readyReadSlot()));
    q->connect(plainSocket, SIGNAL(bytesWritten(qint64)),
               q, SLOT(_q_bytesWrittenSlot(qint64)));
    
    readBuffer.clear();
    writeBuffer.clear();
    connectionEncrypted = false;
    peerCertificate.clear();
    peerCertificateChain.clear();
    mode = QSslSocket::UnencryptedMode;
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_connectedSlot()
{
    Q_Q(QSslSocket);
    q->setLocalPort(plainSocket->localPort());
    q->setLocalAddress(plainSocket->localAddress());
    q->setPeerPort(plainSocket->peerPort());
    q->setPeerAddress(plainSocket->peerAddress());
    q->setPeerName(plainSocket->peerName());
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_connectedSlot()";
    qDebug() << "\tstate =" << q->state();
    qDebug() << "\tpeer =" << q->peerName() << q->peerAddress() << q->peerPort();
    qDebug() << "\tlocal =" << QHostInfo::fromName(q->localAddress().toString()).hostName()
             << q->localAddress() << q->localPort();
#endif
    emit q->connected();

    if (autoStartHandshake)
        q->startClientEncryption();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_hostFoundSlot()
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_hostFoundSlot()";
    qDebug() << "\tstate =" << q->state();
#endif
    emit q->hostFound();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_disconnectedSlot()
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_disconnectedSlot()";
    qDebug() << "\tstate =" << q->state();
#endif
    disconnected();
    emit q->disconnected();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_stateChangedSlot(QAbstractSocket::SocketState state)
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_stateChangedSlot(" << state << ")";
#endif
    q->setSocketState(state);
    emit q->stateChanged(state);
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_errorSlot(QAbstractSocket::SocketError error)
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_errorSlot(" << error << ")";
    qDebug() << "\tstate =" << q->state();
    qDebug() << "\terrorString =" << q->errorString();
#endif
    q->setSocketError(plainSocket->error());
    q->setErrorString(plainSocket->errorString());
    emit q->error(error);
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_readyReadSlot()
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_readyReadSlot() -" << plainSocket->bytesAvailable() << "bytes available";
#endif
    if (mode == QSslSocket::UnencryptedMode) {
        emit q->readyRead();
        return;
    }

    transmit();
}

/*!
    \internal
*/
void QSslSocketPrivate::_q_bytesWrittenSlot(qint64 written)
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_bytesWrittenSlot(" << written << ")";
#endif
    emit q->bytesWritten(written);
}

// For private slots
#define d d_ptr
#include "moc_qsslsocket.cpp"
