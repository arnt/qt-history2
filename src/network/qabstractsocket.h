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

class QM_EXPORT_NETWORK QAbstractSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    QAbstractSocket(Qt::SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    virtual bool connectToHost(const QString &hostName, Q_UINT16 port);
    bool connectToHost(const QHostAddress &address, Q_UINT16 port);

    bool isBlocking() const;
    void setBlocking(bool blocking, int msec = 30000);

    virtual bool isValid() const;

    virtual Q_LLONG bytesAvailable() const;
    virtual Q_LLONG bytesToWrite() const;

    virtual bool canReadLine() const;
    virtual QByteArray readLine();

    Q_UINT16 localPort() const;
    QHostAddress localAddress() const;
    Q_UINT16 peerPort() const;
    QHostAddress peerAddress() const;
    QString peerName() const;

    virtual Q_LLONG readBufferSize() const;
    virtual void setReadBufferSize(Q_LLONG size);

    virtual bool waitForReadyRead(int msecs = 30000);

    virtual void abort();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             Qt::SocketState state = Qt::ConnectedState);

    Qt::SocketState socketState() const;
    Qt::SocketType socketType() const;

    Qt::SocketError socketError() const;
    QString errorString() const;

    // from QIODevice

    bool open(int);
    void close();
    void flush();

    Q_LLONG size() const;
    Q_LLONG at() const;
    bool seek(Q_LLONG offset);

    Q_LLONG read(char *data, Q_LLONG maxlen);
    Q_LLONG write(const char *data, Q_LLONG len);
    Q_LLONG readLine(char *data, Q_LLONG maxlen);
    QByteArray readAll();

    int getch();
    int putch(int character);
    int ungetch(int character);

signals:
    void hostFound();
    void connected();
    void closing();
    void closed();
    void readyRead();
    void stateChanged(int);
    void bytesWritten(Q_LLONG nbytes);
    void error(int);

protected:
    QAbstractSocket(Qt::SocketType socketType,
                    QAbstractSocketPrivate &p, QObject *parent);

    QAbstractSocketPrivate *d_ptr;

    void initSocketNotifiers();

    void setSocketType(Qt::SocketType socketType);

private:
    Q_DECLARE_PRIVATE(QAbstractSocket);

    Q_PRIVATE_SLOT(d, void connectToNextAddress());
    Q_PRIVATE_SLOT(d, void startConnecting(const QDnsHostInfo &));
    Q_PRIVATE_SLOT(d, void testConnection());
    Q_PRIVATE_SLOT(d, void canReadNotification(int));
    Q_PRIVATE_SLOT(d, void canWriteNotification(int));
};

#endif
