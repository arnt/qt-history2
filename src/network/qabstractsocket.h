/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTSOCKET_H
#define QABSTRACTSOCKET_H

namespace Qt {
    enum SocketType {
        TcpSocket,
        UdpSocket,
        UnknownSocketType = -1
    };
    enum NetworkLayerProtocol {
        IPv4Protocol,
        IPv6Protocol,
        UnknownNetworkLayerProtocol = -1
    };
    enum SocketError {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        DatagramTooLargeError,
        NetworkError,
        AddressInUseError,
        SocketAddressNotAvailableError,
        UnsupportedSocketOperationError,
        UnknownSocketError = -1
    };
    enum SocketState {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ListeningState,
        ClosingState
    };
}

#include <qiodevice.h>
#include <qobject.h>

class QHostAddress;
class QAbstractSocketPrivate;

class Q_NETWORK_EXPORT QAbstractSocket : public QIODevice
{
    Q_OBJECT
public:
    QAbstractSocket(Qt::SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    void connectToHost(const QString &hostName, Q_UINT16 port, OpenMode mode = ReadWrite);
    void connectToHost(const QHostAddress &address, Q_UINT16 port, OpenMode mode = ReadWrite);
    void disconnectFromHost();

    bool isValid() const;

    Q_LONGLONG bytesAvailable() const;
    Q_LONGLONG bytesToWrite() const;

    bool canReadLine() const;

    Q_UINT16 localPort() const;
    QHostAddress localAddress() const;
    Q_UINT16 peerPort() const;
    QHostAddress peerAddress() const;
    QString peerName() const;

    Q_LONGLONG readBufferSize() const;
    void setReadBufferSize(Q_LONGLONG size);

    void abort();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor, Qt::SocketState state = Qt::ConnectedState,
                             OpenMode openMode = ReadWrite);

    Qt::SocketType socketType() const;
    Qt::SocketState socketState() const;
    Qt::SocketError socketError() const;

    // from QIODevice
    void close();
    bool isSequential() const;

    // for synchronous access
    bool waitForConnected(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

signals:
    void hostFound();
    void connected();
    void disconnected();
    void stateChanged(int);
    void error(int);

protected:
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

    void setSocketState(Qt::SocketState state);
    void setSocketError(Qt::SocketError socketError);

protected:
    QAbstractSocket(Qt::SocketType socketType, QAbstractSocketPrivate &p, QObject *parent);

private:
    Q_DECLARE_PRIVATE(QAbstractSocket)
    Q_DISABLE_COPY(QAbstractSocket)

    Q_PRIVATE_SLOT(d, void connectToNextAddress())
    Q_PRIVATE_SLOT(d, void startConnecting(const QDnsHostInfo &))
    Q_PRIVATE_SLOT(d, void abortConnectionAttempt())
    Q_PRIVATE_SLOT(d, void testConnection())
    Q_PRIVATE_SLOT(d, bool canReadNotification(int))
    Q_PRIVATE_SLOT(d, bool canWriteNotification(int))

#ifdef QT_COMPAT
public:
    enum Error {
        ErrConnectionRefused = Qt::ConnectionRefusedError,
        ErrHostNotFound = Qt::HostNotFoundError,
        ErrSocketRead = Qt::UnknownSocketError
    };
    enum State {
        Idle = Qt::UnconnectedState,
        HostLookup = Qt::HostLookupState,
        Connecting = Qt::ConnectingState,
        Connected = Qt::ConnectedState,
        Closing = Qt::ClosingState,
        Connection = Qt::ConnectedState
    };
    inline QT_COMPAT State state() const { return State(socketState()); }
    inline QT_COMPAT int socket() const { return socketDescriptor(); }
    inline QT_COMPAT void setSocket(int socket) { setSocketDescriptor(socket); }
    inline QT_COMPAT Q_ULONG waitForMore(int msecs, bool *timeout = 0) const
    {
        QAbstractSocket *that = const_cast<QAbstractSocket *>(this);
        if (that->waitForReadyRead(msecs))
            return Q_ULONG(bytesAvailable());
        if (socketError() == Qt::SocketTimeoutError && timeout)
            *timeout = true;
        return 0;
    }
signals:
    QT_MOC_COMPAT void connectionClosed(); // same as disconnected()
    QT_MOC_COMPAT void delayedCloseFinished(); // same as disconnected()


#endif
};

#endif
