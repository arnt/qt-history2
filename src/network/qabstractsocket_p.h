#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H
#include "private/qiodevice_p.h"
#include "qabstractsocket.h"
#include "qbytearray.h"
#include "qdns.h"
#include "qlist.h"
#include "qsocketlayer.h"
#include "qsocketnotifier.h"
#include "qtimer.h"

class QRingBuffer
{
public:
    QRingBuffer(int growth = 4096);

    int nextDataBlockSize() const;
    char *readPointer() const;
    void free(int bytes);
    char *reserve(int bytes);
    void truncate(int bytes);

    bool isEmpty() const;

    int getChar();
    void putChar(char c);
    void ungetChar(char c);

    int size() const;
    void clear();
    int indexOf(char c) const;
    int readLine(char *data, int maxLength);
    bool canReadLine() const;

private:
    QList<QByteArray> buffers;
    int head, tail;
    int tailBuffer;
    int basicBlockSize;
    int bufferSize;
};

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
    void flush();

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
    QString socketErrorString;
};

#endif
