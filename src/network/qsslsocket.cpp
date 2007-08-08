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

QSslSocket::~QSslSocket()
{
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::~QSslSocket(), this =" << (void *)this;
#endif
}

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
    setLocalPort(d->plainSocket->localPort());
    setLocalAddress(d->plainSocket->localAddress());
    setPeerPort(d->plainSocket->peerPort());
    setPeerAddress(d->plainSocket->peerAddress());
    setPeerName(d->plainSocket->peerName());
    return retVal;
}

QSslSocket::SslMode QSslSocket::mode() const
{
    Q_D(const QSslSocket);
    return d->mode;
}

bool QSslSocket::isEncrypted() const
{
    Q_D(const QSslSocket);
    return d->connectionEncrypted;
}

QSsl::SslProtocol QSslSocket::protocol() const
{
    Q_D(const QSslSocket);
    return d->protocol;
}

void QSslSocket::setProtocol(QSsl::SslProtocol protocol)
{
    Q_D(QSslSocket);
    d->protocol = protocol;
}

qint64 QSslSocket::bytesAvailable() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::bytesAvailable() + (d->plainSocket ? d->plainSocket->bytesAvailable() : 0);
    return QIODevice::bytesAvailable() + d->readBuffer.size();
}

qint64 QSslSocket::bytesToWrite() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return d->plainSocket ? d->plainSocket->bytesToWrite() : 0;
    return d->writeBuffer.size();
}

bool QSslSocket::canReadLine() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::canReadLine() || (d->plainSocket && d->plainSocket->canReadLine());
    return QIODevice::canReadLine() || (!d->readBuffer.isEmpty() && d->readBuffer.canReadLine());
}

void QSslSocket::close()
{
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::close()";
#endif
    QTcpSocket::close();
}

bool QSslSocket::atEnd() const
{
    Q_D(const QSslSocket);
    if (d->mode == UnencryptedMode)
        return QIODevice::atEnd() && (!d->plainSocket || d->plainSocket->atEnd());
    return QIODevice::atEnd() && d->readBuffer.isEmpty();
}

bool QSslSocket::flush()
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::flush()";
#endif
    return d->plainSocket ? d->plainSocket->flush() : false;
}

void QSslSocket::abort()
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::abort()";
#endif
    if (d->plainSocket)
        d->plainSocket->abort();
}

void QSslSocket::setLocalCertificate(const QSslCertificate &certificate)
{
    Q_D(QSslSocket);
    d->localCertificate = certificate;
}

void
QSslSocket::setLocalCertificate(const QString &path,
				QSsl::EncodingFormat format)
{
    Q_D(QSslSocket);
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        d->localCertificate = QSslCertificate(file.readAll(), format);
}

QSslCertificate QSslSocket::localCertificate() const
{
    Q_D(const QSslSocket);
    return d->localCertificate;
}

QSslCertificate QSslSocket::peerCertificate() const
{
    Q_D(const QSslSocket);
    return d->peerCertificate;
}

QList<QSslCertificate> QSslSocket::peerCertificateChain() const
{
    Q_D(const QSslSocket);
    return d->peerCertificateChain;
}

QSslCipher QSslSocket::sessionCipher() const
{
    Q_D(const QSslSocket);
    return d->sessionCipher();
}

void QSslSocket::setPrivateKey(const QSslKey &key)
{
    Q_D(QSslSocket);
    d->privateKey = key;
}

void QSslSocket::setPrivateKey(const QString &fileName,
			       QSsl::KeyAlgorithm algorithm,
                               QSsl::EncodingFormat format,
                               const QByteArray &passPhrase)
{
    Q_D(QSslSocket);
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        d->privateKey = QSslKey(file.readAll(),
				algorithm,
				format,
                                QSsl::PrivateKey,
				passPhrase);
    }
}

QSslKey QSslSocket::privateKey() const
{
    Q_D(const QSslSocket);
    return d->privateKey;
}

QList<QSslCipher> QSslSocket::ciphers() const
{
    Q_D(const QSslSocket);
    return d->ciphers;
}

void QSslSocket::setCiphers(const QList<QSslCipher> &ciphers)
{
    Q_D(QSslSocket);
    d->ciphers = ciphers;
}

void QSslSocket::setCiphers(const QString &ciphers)
{
    Q_D(QSslSocket);
    d->ciphers.clear();
    foreach (QString cipherName,
	     ciphers.split(QLatin1String(":"),QString::SkipEmptyParts)) {
        for (int i = 0; i < 3; ++i) {
            // ### Crude
            QSslCipher cipher(cipherName, QSsl::SslProtocol(i));
            if (!cipher.isNull())
                d->ciphers << cipher;
        }
    }
}

void QSslSocket::setDefaultCiphers(const QList<QSslCipher> &ciphers)
{
    QSslSocketPrivate::setDefaultCiphers(ciphers);
}

QList<QSslCipher> QSslSocket::defaultCiphers()
{
    return QSslSocketPrivate::defaultCiphers();
}

QList<QSslCipher> QSslSocket::supportedCiphers()
{
    return QSslSocketPrivate::supportedCiphers();
}

bool QSslSocket::addCaCertificates(const QString &path,
				   QSsl::EncodingFormat format,
                                   QRegExp::PatternSyntax syntax)
{
    Q_D(QSslSocket);
    QList<QSslCertificate> certs = QSslCertificate::fromPath(path,format,syntax);
    if (certs.isEmpty())
        return false;

    d->localCaCertificates += certs;
    return true;
}

void QSslSocket::addCaCertificate(const QSslCertificate &certificate)
{
    Q_D(QSslSocket);
    d->localCaCertificates += certificate;
}

void QSslSocket::addCaCertificates(const QList<QSslCertificate> &certificates)
{
    Q_D(QSslSocket);
    d->localCaCertificates += certificates;
}

void QSslSocket::setCaCertificates(const QList<QSslCertificate> &certificates)
{
    Q_D(QSslSocket);
    d->useLocalCaCertificatesOnly = true;
    d->localCaCertificates = certificates;
}

QList<QSslCertificate> QSslSocket::caCertificates() const
{
    Q_D(const QSslSocket);
    if (d->useLocalCaCertificatesOnly)
        return d->localCaCertificates;
    return d->defaultCaCertificates() + d->localCaCertificates;
}

bool QSslSocket::addDefaultCaCertificates(const QString &path,
					  QSsl::EncodingFormat encoding,
                                          QRegExp::PatternSyntax syntax)
{
    return QSslSocketPrivate::addDefaultCaCertificates(path, encoding, syntax);
}

void QSslSocket::addDefaultCaCertificate(const QSslCertificate &certificate)
{
    QSslSocketPrivate::addDefaultCaCertificate(certificate);
}

void
QSslSocket::addDefaultCaCertificates(const QList<QSslCertificate> &certificates)
{
    QSslSocketPrivate::addDefaultCaCertificates(certificates);
}

void
QSslSocket::setDefaultCaCertificates(const QList<QSslCertificate> &certificates)
{
    QSslSocketPrivate::setDefaultCaCertificates(certificates);
}

QList<QSslCertificate> QSslSocket::defaultCaCertificates()
{
    return QSslSocketPrivate::defaultCaCertificates();
}

QList<QSslCertificate> QSslSocket::systemCaCertificates()
{
    QSslSocketPrivate::ensureInitialized();
    return QSslSocketPrivate::systemCaCertificates();
}

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

QList<QSslError> QSslSocket::sslErrors() const
{
    Q_D(const QSslSocket);
    return d->sslErrors;
}

bool QSslSocket::supportsSsl()
{
    return QSslSocketPrivate::ensureInitialized();
}

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

void QSslSocket::ignoreSslErrors()
{
    Q_D(QSslSocket);
    d->ignoreSslErrors = true;
}

void QSslSocket::connectToHostImplementation(const QString &hostName,
					     quint16 port,
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

qint64 QSslSocket::readData(char *data, qint64 maxlen)
{
    Q_D(QSslSocket);
    qint64 readBytes = 0;

    if (d->mode == UnencryptedMode && !d->autoStartHandshake) {
        readBytes = d->plainSocket->read(data, maxlen);
    } else {
        do {
            const char *readPtr = d->readBuffer.readPointer();
            int bytesToRead = qMin<int>(maxlen - readBytes, d->readBuffer.nextDataBlockSize());
            ::memcpy(data + readBytes, readPtr, bytesToRead);
            readBytes += bytesToRead;
            d->readBuffer.free(bytesToRead);
        } while (!d->readBuffer.isEmpty() && readBytes < maxlen);
    }
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::readData(" << (void *)data << "," << maxlen << ") ==" << readBytes;
#endif
    return readBytes;
}

qint64 QSslSocket::writeData(const char *data, qint64 len)
{
    Q_D(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::writeData(" << (void *)data << "," << len << ")";
#endif
    if (d->mode == UnencryptedMode && !d->autoStartHandshake)
        return d->plainSocket->write(data, len);

    char *writePtr = d->writeBuffer.reserve(len);
    ::memcpy(writePtr, data, len);

    if (d->connectionEncrypted)
        d->transmit();

    return len;
}

QSslSocketPrivate::QSslSocketPrivate()
    : protocol(QSsl::SslV3), plainSocket(0)
{
}

QSslSocketPrivate::~QSslSocketPrivate()
{
}

void QSslSocketPrivate::init()
{
    mode = QSslSocket::UnencryptedMode;
    autoStartHandshake = false;
    connectionEncrypted = false;
    ignoreSslErrors = false;
    useLocalCaCertificatesOnly = false;

    readBuffer.clear();
    writeBuffer.clear();
    peerCertificate.clear();
}

QList<QSslCipher> QSslSocketPrivate::defaultCiphers()
{
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->ciphers;
}

QList<QSslCipher> QSslSocketPrivate::supportedCiphers()
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->supportedCiphers;
}

void QSslSocketPrivate::setDefaultCiphers(const QList<QSslCipher> &ciphers)
{
    QMutexLocker locker(&globalData()->mutex);
    globalData()->ciphers = ciphers;
}

void
QSslSocketPrivate::setDefaultSupportedCiphers(const QList<QSslCipher> &ciphers)
{
    QMutexLocker locker(&globalData()->mutex);
    globalData()->supportedCiphers = ciphers;
}

QList<QSslCertificate> QSslSocketPrivate::defaultCaCertificates()
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    return globalData()->caCertificates;
}

void
QSslSocketPrivate::setDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates = certs;
}

bool
QSslSocketPrivate::addDefaultCaCertificates(const QString &path,
					    QSsl::EncodingFormat format,
					    QRegExp::PatternSyntax syntax)
{
    QSslSocketPrivate::ensureInitialized();
    QList<QSslCertificate> certs=QSslCertificate::fromPath(path,format,syntax);
    if (certs.isEmpty())
        return false;

    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += certs;
    return true;
}

void QSslSocketPrivate::addDefaultCaCertificate(const QSslCertificate &cert)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += cert;
}

void
QSslSocketPrivate::addDefaultCaCertificates(const QList<QSslCertificate> &certs)
{
    QSslSocketPrivate::ensureInitialized();
    QMutexLocker locker(&globalData()->mutex);
    globalData()->caCertificates += certs;
}

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
               q, SLOT(_q_connectedSlot()),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(hostFound()),
               q, SLOT(_q_hostFoundSlot()),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(disconnected()),
               q, SLOT(_q_disconnectedSlot()),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
               q, SLOT(_q_stateChangedSlot(QAbstractSocket::SocketState)),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(error(QAbstractSocket::SocketError)),
               q, SLOT(_q_errorSlot(QAbstractSocket::SocketError)),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(readyRead()),
               q, SLOT(_q_readyReadSlot()),
               Qt::DirectConnection);
    q->connect(plainSocket, SIGNAL(bytesWritten(qint64)),
               q, SLOT(_q_bytesWrittenSlot(qint64)),
               Qt::DirectConnection);
    
    readBuffer.clear();
    writeBuffer.clear();
    connectionEncrypted = false;
    peerCertificate.clear();
    peerCertificateChain.clear();
    mode = QSslSocket::UnencryptedMode;
}

void QSslSocketPrivate::_q_connectedSlot()
{
    Q_Q(QSslSocket);
    q->setLocalPort(plainSocket->localPort());
    q->setLocalAddress(plainSocket->localAddress());
    q->setPeerPort(plainSocket->peerPort());
    q->setPeerAddress(plainSocket->peerAddress());
    q->setPeerName(plainSocket->peerName());
    cachedSocketDescriptor = plainSocket->socketDescriptor();

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

void QSslSocketPrivate::_q_hostFoundSlot()
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_hostFoundSlot()";
    qDebug() << "\tstate =" << q->state();
#endif
    emit q->hostFound();
}

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

void QSslSocketPrivate::_q_stateChangedSlot(QAbstractSocket::SocketState state)
{
    Q_Q(QSslSocket);
#ifdef QSSLSOCKET_DEBUG
    qDebug() << "QSslSocket::_q_stateChangedSlot(" << state << ")";
#endif
    q->setSocketState(state);
    emit q->stateChanged(state);
}

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
