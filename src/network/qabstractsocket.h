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

class QM_EXPORT_NETWORK QAbstractSocket : public QObject, public QIODevice
{
    Q_OBJECT
public:
    QAbstractSocket(Qt::SocketType socketType, QObject *parent);
    virtual ~QAbstractSocket();

    virtual QIODevice::DeviceType deviceType() const { return castDeviceType(); }
    static QIODevice::DeviceType castDeviceType() { return QIODevice::IOType_QAbstractSocket; }

    virtual bool connectToHost(const QString &hostName, Q_UINT16 port);
    bool connectToHost(const QHostAddress &address, Q_UINT16 port);

    virtual bool isValid() const;

    virtual Q_LONGLONG bytesAvailable() const;
    virtual Q_LONGLONG bytesToWrite() const;

    virtual bool canReadLine() const;
    virtual QByteArray readLine();

    Q_UINT16 localPort() const;
    QHostAddress localAddress() const;
    Q_UINT16 peerPort() const;
    QHostAddress peerAddress() const;
    QString peerName() const;

    virtual Q_LONGLONG readBufferSize() const;
    virtual void setReadBufferSize(Q_LONGLONG size);

    virtual bool waitForReadyRead(int msecs = 30000);

    virtual void abort();

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor,
                             Qt::SocketState state = Qt::ConnectedState);

    Qt::SocketState socketState() const;
    Qt::SocketType socketType() const;

    Qt::SocketError socketError() const;
    QString errorString() const;

    bool isBlocking() const;
    void setBlocking(bool blocking, int msec = 30000);

    // from QIODevice

    bool isOpen() const;

    bool open(int);
    void close();
    void flush();

    Q_LONGLONG size() const;
    Q_LONGLONG at() const;
    bool seek(Q_LONGLONG offset);

    Q_LONGLONG read(char *data, Q_LONGLONG maxlen);

    Q_LONGLONG write(const char *data, Q_LONGLONG len);
#if !defined(Q_NO_USING_KEYWORD)
    using QIODevice::write;
#else
    inline Q_LONGLONG write(const QByteArray &ba) { return QIODevice::write(ba); }
#endif
    Q_LONGLONG readLine(char *data, Q_LONGLONG maxlen);
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
    void bytesWritten(Q_LONGLONG nbytes);
    void error(int);

protected:
    QAbstractSocket(Qt::SocketType socketType,
                    QAbstractSocketPrivate &p, QObject *parent);

    QAbstractSocketPrivate *d_ptr;

private:
    inline int state() {return 0;} //to help catch programming errors: socketState() is the function you want

    Q_DECLARE_PRIVATE(QAbstractSocket)
    Q_DISABLE_COPY(QAbstractSocket)

    Q_PRIVATE_SLOT(d, void connectToNextAddress());
    Q_PRIVATE_SLOT(d, void startConnecting(const QDnsHostInfo &));
    Q_PRIVATE_SLOT(d, void abortConnectionAttempt());
    Q_PRIVATE_SLOT(d, void testConnection());
    Q_PRIVATE_SLOT(d, void canReadNotification(int));
    Q_PRIVATE_SLOT(d, void canWriteNotification(int));
};

#endif
