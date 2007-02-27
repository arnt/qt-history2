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

#include "qhttpsocketengine_p.h"
#include "qtcpsocket.h"
#include "qhostaddress.h"
#include "qdatetime.h"
#include "qhttp.h"

#include <qdebug.h>

#define DEBUG

QHttpSocketEngine::QHttpSocketEngine(QObject *parent)
    : QAbstractSocketEngine(*new QHttpSocketEnginePrivate, parent)
{
}

QHttpSocketEngine::~QHttpSocketEngine()
{
}

bool QHttpSocketEngine::initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol)
{
    Q_D(QHttpSocketEngine);
    if (type != QAbstractSocket::TcpSocket)
        return false;

    setProtocol(protocol);
    setSocketType(type);
    d->socket = new QTcpSocket(this);

#ifndef QT_NO_NETWORKPROXY
    // Explicitly disable proxying on the proxy socket itself to avoid
    // unwanted recursion.
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    d->socket->setProxy(proxy);
#endif

    // Intercept all the signals.
    connect(d->socket, SIGNAL(connected()),
            this, SLOT(slotSocketConnected()));
    connect(d->socket, SIGNAL(disconnected()),
            this, SLOT(slotSocketDisconnected()));
    connect(d->socket, SIGNAL(readyRead()),
            this, SLOT(slotSocketReadNotification()));
    connect(d->socket, SIGNAL(readyRead()),
            this, SLOT(slotSocketReadNotification()));
    connect(d->socket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(slotSocketBytesWritten()));
    connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotSocketError(QAbstractSocket::SocketError)));
    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(slotSocketStateChanged(QAbstractSocket::SocketState)));

    return true;
}

bool QHttpSocketEngine::initialize(int, QAbstractSocket::SocketState)
{
    return false;
}

#ifndef QT_NO_NETWORKPROXY
void QHttpSocketEngine::setProxy(const QNetworkProxy &proxy)
{
    Q_D(QHttpSocketEngine);
    d->proxy = proxy;
    QString user = proxy.user();
    if (!user.isEmpty())
        d->authenticator.setUser(user);
    QString password = proxy.password();
    if (!password.isEmpty())
        d->authenticator.setPassword(password);
}
#endif

int QHttpSocketEngine::socketDescriptor() const
{
    Q_D(const QHttpSocketEngine);
    return d->socket ? d->socket->socketDescriptor() : 0;
}

bool QHttpSocketEngine::isValid() const
{
    Q_D(const QHttpSocketEngine);
    return d->socket;
}

bool QHttpSocketEngine::connectToHost(const QHostAddress &address, quint16 port)
{
    Q_D(QHttpSocketEngine);

    // If the handshake is done, enter ConnectedState state and return true.
    if (d->state == Connected) {
        setState(QAbstractSocket::ConnectedState);
        return true;
    }

    if (d->state == ConnectSent && d->socketState != QAbstractSocket::ConnectedState)
        setState(QAbstractSocket::UnconnectedState);

    // Handshake isn't done. If unconnected, start connecting.
    if (d->state == None && d->socket->state() == QAbstractSocket::UnconnectedState) {
        setPeerAddress(address);
        setPeerPort(port);
        setState(QAbstractSocket::ConnectingState);
#ifndef QT_NO_NETWORKPROXY
        d->socket->connectToHost(d->proxy.hostName(), d->proxy.port());
#endif
    }

    // If connected (might happen right away, at least for localhost services
    // on some BSD systems), there might already be bytes available.
    if (bytesAvailable())
        slotSocketReadNotification();

    return d->socketState == QAbstractSocket::ConnectedState;
}

bool QHttpSocketEngine::bind(const QHostAddress &, quint16)
{
    return false;
}

bool QHttpSocketEngine::listen()
{
    return false;
}

int QHttpSocketEngine::accept()
{
    return 0;
}

void QHttpSocketEngine::close()
{
    Q_D(QHttpSocketEngine);
    if (d->socket)
        d->socket->close();
}

qint64 QHttpSocketEngine::bytesAvailable() const
{
    Q_D(const QHttpSocketEngine);
    return d->readBuffer.size() + (d->socket ? d->socket->bytesAvailable() : 0);
}

qint64 QHttpSocketEngine::read(char *data, qint64 maxlen)
{
    Q_D(QHttpSocketEngine);
    qint64 bytesRead = 0;

    if (!d->readBuffer.isEmpty()) {
        // Read as much from the buffer as we can.
        bytesRead = qMin((qint64)d->readBuffer.size(), maxlen);
        memcpy(data, d->readBuffer.constData(), bytesRead);
        data += bytesRead;
        maxlen -= bytesRead;
        d->readBuffer = d->readBuffer.mid(bytesRead);
    }

    qint64 bytesReadFromSocket = d->socket->read(data, maxlen);
    if (bytesReadFromSocket > 0) {
        // Add to what we read so far.
        bytesRead += bytesReadFromSocket;
    } else if (bytesRead == 0) {
        // If nothing has been read so far, and the direct socket read
        // failed, return the socket's error. Otherwise, fall through and
        // return as much as we read so far.
        close();
        setError(QAbstractSocket::RemoteHostClosedError,
                 QLatin1String("Remote host closed"));
        setState(QAbstractSocket::UnconnectedState);
        return -1;
    }
    return bytesRead;
}

qint64 QHttpSocketEngine::write(const char *data, qint64 len)
{
    Q_D(QHttpSocketEngine);
    return d->socket->write(data, len);
}

#ifndef QT_NO_UDPSOCKET
qint64 QHttpSocketEngine::readDatagram(char *, qint64, QHostAddress *,
                                       quint16 *)
{
    return 0;
}

qint64 QHttpSocketEngine::writeDatagram(const char *, qint64, const QHostAddress &,
                                        quint16)
{
    return 0;
}

bool QHttpSocketEngine::hasPendingDatagrams() const
{
    return false;
}

qint64 QHttpSocketEngine::pendingDatagramSize() const
{
    return 0;
}
#endif // QT_NO_UDPSOCKET

int QHttpSocketEngine::option(SocketOption) const
{
    return -1;
}

bool QHttpSocketEngine::setOption(SocketOption, int)
{
    return false;
}

/*
   Returns the difference between msecs and elapsed. If msecs is -1,
   however, -1 is returned.
*/
static int qt_timeout_value(int msecs, int elapsed)
{
    if (msecs == -1)
        return -1;

    int timeout = msecs - elapsed;
    return timeout < 0 ? 0 : timeout;
}

bool QHttpSocketEngine::waitForRead(int msecs, bool *timedOut) const
{
    Q_D(const QHttpSocketEngine);

    if (!d->socket || d->socket->state() == QAbstractSocket::UnconnectedState)
        return false;

    QTime stopWatch;
    stopWatch.start();

    // Wait for more data if nothing is available.
    if (!d->socket->bytesAvailable()) {
        if (!d->socket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
            if (d->socket->state() == QAbstractSocket::UnconnectedState)
                return true;
            setError(d->socket->error(), d->socket->errorString());
            if (timedOut && d->socket->error() == QAbstractSocket::SocketTimeoutError)
                *timedOut = true;
            return false;
        }
    }

    // If we're not connected yet, wait until we are, or until an error
    // occurs.
    while (d->state != Connected && d->socket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed()))) {
        // Loop while the protocol handshake is taking place.
    }

    // Report any error that may occur.
    if (d->state != Connected) {
        setError(d->socket->error(), d->socket->errorString());
        if (timedOut && d->socket->error() == QAbstractSocket::SocketTimeoutError)
            *timedOut = true;
        return false;
    }
    return true;
}

bool QHttpSocketEngine::waitForWrite(int msecs, bool *timedOut) const
{
    Q_D(const QHttpSocketEngine);

    // If we're connected, just forward the call.
    if (d->state == Connected) {
        if (d->socket->bytesToWrite()) {
            if (!d->socket->waitForBytesWritten(msecs)) {
                if (d->socket->error() == QAbstractSocket::SocketTimeoutError && timedOut)
                    *timedOut = true;
                return false;
            }
        }
        return true;
    }

    QTime stopWatch;
    stopWatch.start();

    // If we're not connected yet, wait until we are, and until bytes have
    // been received (i.e., the socket has connected, we have sent the
    // greeting, and then received the response).
    if (d->socket->state() != QAbstractSocket::ConnectedState) {
        if (!d->socket->waitForReadyRead(qt_timeout_value(msecs, stopWatch.elapsed())))
            return false;
    }

    // Report any error that may occur.
    if (d->state != Connected) {
        setError(d->socket->error(), d->socket->errorString());
        if (timedOut && d->socket->error() == QAbstractSocket::SocketTimeoutError)
            *timedOut = true;
    }

    return true;
}

bool QHttpSocketEngine::waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                                           bool checkRead, bool checkWrite,
                                           int msecs, bool *timedOut) const
{
    Q_UNUSED(checkRead);

    if (!checkWrite) {
        // Not interested in writing? Then we wait for read notifications.
        bool canRead = waitForRead(msecs, timedOut);
        if (readyToRead)
            *readyToRead = canRead;
        return canRead;
    }

    // Interested in writing? Then we wait for write notifications.
    bool canWrite = waitForWrite(msecs, timedOut);
    if (readyToWrite)
        *readyToWrite = canWrite;
    return canWrite;
}

bool QHttpSocketEngine::isReadNotificationEnabled() const
{
    Q_D(const QHttpSocketEngine);
    return d->readNotificationEnabled;
}

void QHttpSocketEngine::setReadNotificationEnabled(bool enable)
{
    Q_D(QHttpSocketEngine);
    if (d->readNotificationEnabled == enable)
        return;

    d->readNotificationEnabled = enable;
    if (enable) {
        // Enabling read notification can trigger a notification.
        if (bytesAvailable())
            slotSocketReadNotification();
    }
}

bool QHttpSocketEngine::isWriteNotificationEnabled() const
{
    Q_D(const QHttpSocketEngine);
    return d->writeNotificationEnabled;
}

void QHttpSocketEngine::setWriteNotificationEnabled(bool enable)
{
    Q_D(QHttpSocketEngine);
    d->writeNotificationEnabled = enable;
    if (enable && d->state == Connected && d->socket->state() == QAbstractSocket::ConnectedState)
        QMetaObject::invokeMethod(this, "writeNotification", Qt::QueuedConnection);
}

bool QHttpSocketEngine::isExceptionNotificationEnabled() const
{
    Q_D(const QHttpSocketEngine);
    return d->exceptNotificationEnabled;
}

void QHttpSocketEngine::setExceptionNotificationEnabled(bool enable)
{
    Q_D(QHttpSocketEngine);
    d->exceptNotificationEnabled = enable;
}

void QHttpSocketEngine::slotSocketConnected()
{
    Q_D(QHttpSocketEngine);

    // Send the greeting.
    QByteArray data = "CONNECT ";
    data += d->peerAddress.toString().toLatin1() + ':' + QByteArray::number(d->peerPort);
    data += " HTTP/1.1\r\n";
    QByteArray request = data;
    data += "Proxy-Connection: keep-alive\r\n"
            "Host: " + d->peerAddress.toString().toLatin1() + "\r\n";
    QAuthenticatorPrivate *priv = QAuthenticatorPrivate::getPrivate(d->authenticator);
    //qDebug() << "slotSocketConnected: priv=" << priv << (priv ? (int)priv->method : -1);
    if (priv && priv->method != QAuthenticatorPrivate::None) {
        data += "Proxy-Authorization: " + priv->calculateResponse(data);
        data += "\r\n";
    }
    data += "\r\n";
//     qDebug() << ">>>>>>>> sending request" << this;
//     qDebug() << data;
//     qDebug() << ">>>>>>>";
    d->socket->write(data);
    d->state = ConnectSent;
}

void QHttpSocketEngine::slotSocketDisconnected()
{
    Q_D(QHttpSocketEngine);
    if (d->state != SendAuthentication)
        setState(QAbstractSocket::UnconnectedState);
}

void QHttpSocketEngine::slotSocketReadNotification()
{
    Q_D(QHttpSocketEngine);
    if (d->state != Connected && d->socket->bytesAvailable() == 0)
        return;

    if (d->state == Connected) {
        // Forward as a read notification.
        if (d->readNotificationEnabled)
            emitReadNotification();
        return;
    }

  readResponseContent:
    if (d->state == ReadResponseContent) {
        char dummybuffer[4096];
        while (d->pendingResponseData) {
            int read = d->socket->read(dummybuffer, qMin(sizeof(dummybuffer), (size_t)d->pendingResponseData));
            dummybuffer[read] = 0;

            if (read == 0)
                return;
            if (read == -1) {
                d->socket->disconnectFromHost();
                emitWriteNotification();
                return;
            }
            d->pendingResponseData -= read;
        }
        if (d->pendingResponseData > 0)
            return;
        d->state = SendAuthentication;
        slotSocketConnected();
        return;
    }

    // Still in handshake mode. Wait until we've got a full response.
    bool done = false;
    do {
        d->readBuffer += d->socket->readLine();
    } while (!(done = d->readBuffer.endsWith("\r\n\r\n")) && d->socket->canReadLine());

    if (!done) {
        // Wait for more.
        return;
    }

    if (d->readBuffer.startsWith("HTTP/1.0 200")) {
        d->state = Connected;
        d->readBuffer.clear();
    } else if (d->readBuffer.startsWith("HTTP/1.0 503")) {
        // 503 Service Unavailable: Connection Refused
        d->socket->close();
        setState(QAbstractSocket::UnconnectedState);
        setError(QAbstractSocket::ConnectionRefusedError, QAbstractSocket::tr("Connection refused"));
    } else if (d->readBuffer.startsWith("HTTP/1.0 407")) {
        if (d->authenticator.isNull())
            d->authenticator.detach();
        QAuthenticatorPrivate *priv = QAuthenticatorPrivate::getPrivate(d->authenticator);

        QHttpResponseHeader responseHeader(QString::fromLatin1(d->readBuffer));
        priv->parseHttpResponse(responseHeader, true);

//         qDebug(">>>>>>>>>>>>>%x: GOT 407: [%s] method=%d, realm=%s challenge=%s", this, d->readBuffer.trimmed().data(),
//                  priv->method, priv->realm.toLatin1().data(), priv->challenge.data());
//         qDebug() << "d->state=" << d->state << "socketState=" << d->socketState << "phase=" << priv->phase;

        d->readBuffer.clear();
        if (priv->phase == QAuthenticatorPrivate::Done) 
            emit proxyAuthenticationRequired(&d->authenticator);

        // priv->phase will get reset to QAuthenticatorPrivate::Start if the authenticator got modified in the signal above.
        if (priv->phase == QAuthenticatorPrivate::Done) {
            setError(QAbstractSocket::ProxyAuthenticationRequiredError, tr("Authentication required"));
            d->socket->disconnectFromHost();
        } else {
            // close the connection if it isn't already and reconnect using the chose authentication method
            d->state = SendAuthentication;
            bool willClose = (responseHeader.value(QLatin1String("Proxy-Connection")).toLower() == QLatin1String("close"));
            if (willClose) {
                d->socket->disconnectFromHost();
                d->socket->readAll();
#ifndef QT_NO_NETWORKPROXY
                d->socket->connectToHost(d->proxy.hostName(), d->proxy.port());
#endif
            } else {
                bool ok;
                int contentLength = responseHeader.value(QLatin1String("Content-Length")).toInt(&ok);
                if (ok && contentLength > 0) {
                    d->state = ReadResponseContent;
                    d->pendingResponseData = contentLength;
                    goto readResponseContent;
                }
            }
            return;
        }
    } else {
        qWarning("UNEXPECTED RESPONSE: [%s]", d->readBuffer.trimmed().data());
        d->socket->disconnectFromHost();
    }

    // The handshake is done; request a new connection attempt by sending a write
    // notification.
    emitWriteNotification();
}

void QHttpSocketEngine::slotSocketBytesWritten()
{
    Q_D(QHttpSocketEngine);
    if (d->state == Connected && d->writeNotificationEnabled)
        emitWriteNotification();
}

void QHttpSocketEngine::slotSocketError(QAbstractSocket::SocketError error)
{
    Q_D(QHttpSocketEngine);
    d->readBuffer.clear();
    if (d->state == SendAuthentication || d->state == ReadResponseContent)
        return;
    d->state = None;
    setError(error, d->socket->errorString());
    if (error == QAbstractSocket::RemoteHostClosedError)
        emitReadNotification();
}

void QHttpSocketEngine::slotSocketStateChanged(QAbstractSocket::SocketState state)
{
    Q_UNUSED(state);
}

void QHttpSocketEngine::emitPendingReadNotification()
{
    Q_D(QHttpSocketEngine);
    d->readNotificationPending = false;
    if (d->readNotificationEnabled)
        emit readNotification();
}

void QHttpSocketEngine::emitPendingWriteNotification()
{
    Q_D(QHttpSocketEngine);
    d->writeNotificationPending = false;
    if (d->writeNotificationEnabled)
        emit writeNotification();
}

void QHttpSocketEngine::emitReadNotification()
{
    Q_D(QHttpSocketEngine);
    d->readNotificationActivated = true;
    if (d->readNotificationEnabled && !d->readNotificationPending) {
        d->readNotificationPending = true;
        QMetaObject::invokeMethod(this, "emitPendingReadNotification", Qt::QueuedConnection);
    }
}

void QHttpSocketEngine::emitWriteNotification()
{
    Q_D(QHttpSocketEngine);
    d->writeNotificationActivated = true;
    if (d->writeNotificationEnabled && !d->writeNotificationPending) {
        d->writeNotificationPending = true;
        QMetaObject::invokeMethod(this, "emitPendingWriteNotification", Qt::QueuedConnection);
    }
}

QHttpSocketEnginePrivate::QHttpSocketEnginePrivate()
    : readNotificationEnabled(false)
    , writeNotificationEnabled(false)
    , exceptNotificationEnabled(false)
    , readNotificationActivated(false)
    , writeNotificationActivated(false)
    , readNotificationPending(false)
    , writeNotificationPending(false)
{
    socket = 0;
    state = QHttpSocketEngine::None;
}

QHttpSocketEnginePrivate::~QHttpSocketEnginePrivate()
{
}

QAbstractSocketEngine *QHttpSocketEngineHandler::createSocketEngine(const QHostAddress &address,
                                                                    QAbstractSocket::SocketType socketType,
                                                                    QObject *parent)
{
    if (socketType != QAbstractSocket::TcpSocket)
        return 0;

    if (address == QHostAddress::LocalHost || address == QHostAddress::LocalHostIPv6)
        return 0;

    // find proxy info
    QAbstractSocket *abstractSocket = qobject_cast<QAbstractSocket *>(parent);
    if (!abstractSocket)
        return 0;

#ifndef QT_NO_NETWORKPROXY
    QNetworkProxy proxy = abstractSocket->proxy();
    if (proxy.type() == QNetworkProxy::DefaultProxy)
        proxy = QNetworkProxy::applicationProxy();

    if (proxy.type() != QNetworkProxy::HttpProxy)
        return 0;
#endif

    QHttpSocketEngine *engine = new QHttpSocketEngine(parent);
#ifndef QT_NO_NETWORKPROXY
    engine->setProxy(proxy);
#endif
    return engine;
}

QAbstractSocketEngine *QHttpSocketEngineHandler::createSocketEngine(int, QObject *)
{
    return 0;
}
