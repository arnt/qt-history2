#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H
#include "qabstractsocket.h"

#include <private/qinternal_p.h>
#include <private/qiodevice_p.h>
#include <qbytearray.h>
#include <qdns.h>
#include <qlist.h>
#include <private/qsocketlayer_p.h>
#include <qsocketnotifier.h>
#include <qtimer.h>

class QAbstractSocketPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QAbstractSocket)
public:
    QAbstractSocketPrivate();
    virtual ~QAbstractSocketPrivate();

    // slots
    void connectToNextAddress();
    void startConnecting(const QDnsHostInfo &hostInfo);
    void testConnection();
    void canReadNotification(int);
    void canWriteNotification(int);
    void abortConnectionAttempt();

    bool readSocketNotifierCalled;

    QString hostName;
    Q_UINT16 port;
    QHostAddress host;
    QList<QHostAddress> addresses;

    QSocketLayer socketLayer;

    QSocketNotifier *readSocketNotifier;
    QSocketNotifier *writeSocketNotifier;

    void resetSocketLayer();
    bool flush();

    bool initSocketLayer(Qt::SocketType socketType, Qt::NetworkLayerProtocol protocol);
    void setupSocketNotifiers();
    bool readFromSocket();

    Q_LONGLONG readBufferMaxSize;
    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;

    bool isBuffered;
    bool isBlocking;
    int blockingTimeout;

    QTimer connectTimer;
    int connectTimeElapsed;

    Qt::SocketType socketType;
    Qt::SocketState state;

    Qt::SocketError socketError;
};

#endif
