#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H
#include "qdns.h"
#include "private/qiodevice_p.h"
#include "qbytearray.h"
#include "qlist.h"
#include "qsocketnotifier.h"
#include "qabstractsocket.h"
#include "qsocketlayer.h"

class QAbstractSocketPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QAbstractSocket);
public:
    QAbstractSocketPrivate();
    virtual ~QAbstractSocketPrivate();

    // slots
    void connectToNextAddress();
    void startConnecting(const QDnsHostInfo &hostInfo);
    void testConnection();
    void canReadNotification(int);
    void canWriteNotification(int);

    bool readSocketNotifierCalled;

    QString hostName;
    Q_UINT16 port;
    QHostAddress host;
    QList<QHostAddress> addresses;

    QSocketLayer socketLayer;

    QSocketNotifier *readSocketNotifier;
    QSocketNotifier *writeSocketNotifier;

    void resetSocketLayer();
    void flush();

    bool initSocketLayer(Qt::SocketType socketType, Qt::NetworkLayerProtocol protocol);
    void setupSocketNotifiers();
    bool readFromSocket();

    Q_LLONG readBufferMaxSize;
    QByteArray readBuffer;
    QByteArray writeBuffer;

    bool isBuffered;
    bool isBlocking;
    int blockingTimeout;

    Qt::SocketType socketType;
    Qt::SocketState state;

    Qt::SocketError socketError;
    QString socketErrorString;
};

#endif
