#ifndef QSOCKETLAYERPRIVATE_H
#define QSOCKETLAYERPRIVATE_H
#include "qsocketlayer.h"
#include <qhostaddress.h>

#ifdef Q_OS_WIN
class QWindowsSockInit
{
public:
    QWindowsSockInit();
    ~QWindowsSockInit();
    int version;
};
#endif

class QSocketLayerPrivate
{
public:
    QSocketLayerPrivate();
    ~QSocketLayerPrivate();

    int socketDescriptor;

    Qt::SocketType socketType;
    Qt::NetworkLayerProtocol socketProtocol;
    Qt::SocketState socketState;
    mutable Qt::SocketError socketError;
    mutable QString socketErrorString;

    QHostAddress peerAddress;
    Q_UINT16 peerPort;

    QHostAddress localAddress;
    Q_UINT16 localPort;


#ifdef Q_OS_WIN
    QWindowsSockInit winSock;
#endif

    void setError(Qt::SocketError error, const QString &errorString) const;

    enum SocketOption {
        NonBlockingSocketOption,
        BroadcastSocketOption,
        ReceiveBufferSocketOption,
        SendBufferSocketOption
    };

    // native functions
    int option(SocketOption option) const;
    bool setOption(SocketOption option, int value);

    bool createNewSocket(Qt::SocketType type, Qt::NetworkLayerProtocol protocol);

    bool nativeConnect(const QHostAddress &address, Q_UINT16 port);
    bool nativeBind(const QHostAddress &address, Q_UINT16 port);
    bool nativeListen(int backlog);
    int nativeAccept();
    Q_LLONG nativeBytesAvailable() const;

    bool nativeHasPendingDatagrams() const;
    Q_LLONG nativePendingDatagramSize() const;
    Q_LLONG nativeReceiveDatagram(char *data, Q_LLONG maxLength,
                                  QHostAddress *address, Q_UINT16 *port);
    Q_LLONG nativeSendDatagram(const char *data, Q_LLONG length,
                               const QHostAddress &host, Q_UINT16 port);
    Q_LLONG nativeRead(char *data, Q_LLONG maxLength);
    Q_LLONG nativeWrite(const char *data, Q_LLONG length);
    int nativeSelect(int timeout, bool selectForRead) const;

    void nativeClose();

    bool fetchConnectionParameters();

    QSocketLayer *q;
};

#endif
