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

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QHostAddress;
class QAbstractSocketPrivate;

class QM_EXPORT_NETWORK QAbstractSocket : public QIODevice
{
    Q_OBJECT
public:
    QAbstractSocket(Qt::SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    void connectToHost(const QString &hostName, Q_UINT16 port);
    void connectToHost(const QHostAddress &address, Q_UINT16 port);

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
    bool setSocketDescriptor(int socketDescriptor,
                             Qt::SocketState state = Qt::ConnectedState);

    Qt::SocketType socketType() const;
    Qt::SocketState socketState() const;
    Qt::SocketError socketError() const;

    // from QIODevice
    void close();
    bool flush();
    inline bool isSequential() const { return true; }

    // for synchronous access
    bool waitForReadyRead(int msecs = 30000);
    bool waitForConnected(int msecs = 30000);
    bool waitForClosed(int msecs = 30000);

    void setDefaultTimeout(int msecs);
    int defaultTimeout() const;

signals:
    void hostFound();
    void connected();
    void closing();
    void closed();
    void stateChanged(int);
    void error(int);


protected:
    QAbstractSocket(Qt::SocketType socketType,
                    QAbstractSocketPrivate &p, QObject *parent);

    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

    void setSocketState(Qt::SocketState state);
    void setSocketError(Qt::SocketError socketError);

private:
    inline int state() {return 0;} //to help catch programming errors: socketState() is the function you want

    Q_DECLARE_PRIVATE(QAbstractSocket)
    Q_DISABLE_COPY(QAbstractSocket)

    Q_PRIVATE_SLOT(d, void connectToNextAddress())
    Q_PRIVATE_SLOT(d, void startConnecting(const QDnsHostInfo &))
    Q_PRIVATE_SLOT(d, void abortConnectionAttempt())
    Q_PRIVATE_SLOT(d, void testConnection())
    Q_PRIVATE_SLOT(d, void canReadNotification(int))
    Q_PRIVATE_SLOT(d, void canWriteNotification(int))
};

#endif
