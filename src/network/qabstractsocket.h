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

#include <QtCore/qiodevice.h>
#include <QtCore/qobject.h>

class QHostAddress;
class QAbstractSocketPrivate;

class Q_NETWORK_EXPORT QAbstractSocket : public QIODevice
{
    Q_OBJECT
public:
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
#ifdef QT_COMPAT
        ,
        Idle = UnconnectedState,
        HostLookup = HostLookupState,
        Connecting = ConnectingState,
        Connected = ConnectedState,
        Closing = ClosingState,
        Connection = ConnectedState
#endif
    };

    QAbstractSocket(SocketType socketType, QObject *parent);
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
    bool setSocketDescriptor(int socketDescriptor, SocketState state = ConnectedState,
                             OpenMode openMode = ReadWrite);

    SocketType socketType() const;
    SocketState state() const;
    SocketError error() const;

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
    void stateChanged(SocketState);
    void error(SocketError);

protected:
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

    void setSocketState(SocketState state);
    void setSocketError(SocketError socketError);

protected:
    QAbstractSocket(SocketType socketType, QAbstractSocketPrivate &dd, QObject *parent = 0);

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
        ErrConnectionRefused = ConnectionRefusedError,
        ErrHostNotFound = HostNotFoundError,
        ErrSocketRead = UnknownSocketError
    };
    inline QT_COMPAT int socket() const { return socketDescriptor(); }
    inline QT_COMPAT void setSocket(int socket) { setSocketDescriptor(socket); }
    inline QT_COMPAT Q_ULONG waitForMore(int msecs, bool *timeout = 0) const
    {
        QAbstractSocket *that = const_cast<QAbstractSocket *>(this);
        if (that->waitForReadyRead(msecs))
            return Q_ULONG(bytesAvailable());
        if (error() == SocketTimeoutError && timeout)
            *timeout = true;
        return 0;
    }
    typedef SocketState State;
signals:
    QT_MOC_COMPAT void connectionClosed(); // same as disconnected()
    QT_MOC_COMPAT void delayedCloseFinished(); // same as disconnected()


#endif
};

#endif // QABSTRACTSOCKET_H
