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

#ifndef QSOCKETLAYER_P_H
#define QSOCKETLAYER_P_H
#include <qhostaddress.h>
#include <qabstractsocket.h>

class QHostAddress;
class QSocketLayerPrivate;

class QSocketLayer
{
public:
    QSocketLayer();
    ~QSocketLayer();

    bool initialize(Qt::SocketType type, Qt::NetworkLayerProtocol protocol = Qt::IPv4Protocol);
    bool initialize(int socketDescriptor, Qt::SocketState socketState = Qt::ConnectedState);
    Qt::SocketType socketType() const;
    Qt::NetworkLayerProtocol protocol() const;
    int socketDescriptor() const;

    bool isValid() const;

    bool connectToHost(const QHostAddress &address, Q_UINT16 port);
    bool bind(const QHostAddress &address, Q_UINT16 port);
    bool listen();
    int accept();
    void close();

    Q_LONGLONG bytesAvailable() const;

    Q_LONGLONG read(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG write(const char *data, Q_LONGLONG len);

    Q_LONGLONG readDatagram(char *data, Q_LONGLONG maxlen, QHostAddress *addr = 0,
                            Q_UINT16 *port = 0);
    Q_LONGLONG writeDatagram(const char *data, Q_LONGLONG len, const QHostAddress &addr,
                             Q_UINT16 port);
    bool hasPendingDatagrams() const;
    Q_LONGLONG pendingDatagramSize() const;

    Qt::SocketState socketState() const;

    QHostAddress localAddress() const;
    Q_UINT16 localPort() const;
    QHostAddress peerAddress() const;
    Q_UINT16 peerPort() const;

    Q_LONGLONG receiveBufferSize() const;
    void setReceiveBufferSize(Q_LONGLONG bufferSize);

    Q_LONGLONG sendBufferSize() const;
    void setSendBufferSize(Q_LONGLONG bufferSize);

    bool waitForRead(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForReadOrWrite(bool *readyToRead, bool checkRead, bool checkWrite, int msecs = 30000, bool *timedOut = 0) const;

    Qt::SocketError socketError() const;
    QString errorString() const;

private:
    Q_DISABLE_COPY(QSocketLayer)

    QSocketLayerPrivate *d;
};

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
    Q_LONGLONG nativeBytesAvailable() const;

    bool nativeHasPendingDatagrams() const;
    Q_LONGLONG nativePendingDatagramSize() const;
    Q_LONGLONG nativeReceiveDatagram(char *data, Q_LONGLONG maxLength,
                                     QHostAddress *address, Q_UINT16 *port);
    Q_LONGLONG nativeSendDatagram(const char *data, Q_LONGLONG length,
                                  const QHostAddress &host, Q_UINT16 port);
    Q_LONGLONG nativeRead(char *data, Q_LONGLONG maxLength);
    Q_LONGLONG nativeWrite(const char *data, Q_LONGLONG length);
    int nativeSelect(int timeout, bool selectForRead) const;
    int nativeSelect(int timeout, bool checkRead, bool checkWrite, bool *selectForRead) const;

    void nativeClose();

    bool fetchConnectionParameters();

    QSocketLayer *q;
};

#endif
