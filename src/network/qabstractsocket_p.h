#ifndef QABSTRACTSOCKET_P_H
#define QABSTRACTSOCKET_P_H
#include "qdns.h"
#include "private/qiodevice_p.h"
#include "qbytearray.h"
#include "qlist.h"
#include "qsocketnotifier.h"
#include "qabstractsocket.h"
#include "qsocketlayer.h"

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

class QSlidingWindowBuffer
{
public:
    QSlidingWindowBuffer();

    Q_LLONG readFromFront(char *data, Q_LLONG maxLength);
    Q_LLONG writeToEnd(const char *data, Q_LLONG length);

    Q_LLONG size() const;

    Q_LLONG maximumSize() const;
    void setMaximumSize(Q_LLONG size);

    QByteArray readAll();
    bool contains(char character) const;
    int indexOf(char character) const;

    QByteArray left(int len);
    inline void remove(int bytes) { readFromFront(0, bytes); }
    inline bool isEmpty() const { return size() == 0; }

    void clear();

protected:
    void swapBuffers();

private:
    QByteArray buffers[2];
    int head;
    int tail;
    Q_LLONG maximumBufferSize;
    QByteArray *tailBuffer;
};

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
    QRingBuffer readBuffer;
    QRingBuffer writeBuffer;

    bool isBuffered;
    bool isBlocking;
    int blockingTimeout;

    Qt::SocketType socketType;
    Qt::SocketState state;

    Qt::SocketError socketError;
    QString socketErrorString;
};

#endif
