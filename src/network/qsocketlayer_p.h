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

class QSocketLayerPrivate;

class QSocketLayer
{
public:
    enum SocketOption {
        NonBlockingSocketOption,
        BroadcastSocketOption,
        ReceiveBufferSocketOption,
        SendBufferSocketOption,
        AddressReusable
    };

    QSocketLayer();
    ~QSocketLayer();

    bool initialize(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    bool initialize(int socketDescriptor, QAbstractSocket::SocketState socketState = QAbstractSocket::ConnectedState);
    QAbstractSocket::SocketType socketType() const;
    QAbstractSocket::NetworkLayerProtocol protocol() const;
    int socketDescriptor() const;

    bool isValid() const;

    bool connectToHost(const QHostAddress &address, quint16 port);
    bool bind(const QHostAddress &address, quint16 port);
    bool listen();
    int accept();
    void close();

    qint64 bytesAvailable() const;

    qint64 read(char *data, qint64 maxlen);
    qint64 write(const char *data, qint64 len);

    qint64 readDatagram(char *data, qint64 maxlen, QHostAddress *addr = 0,
                            quint16 *port = 0);
    qint64 writeDatagram(const char *data, qint64 len, const QHostAddress &addr,
                             quint16 port);
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;

    QAbstractSocket::SocketState state() const;

    QHostAddress localAddress() const;
    quint16 localPort() const;
    QHostAddress peerAddress() const;
    quint16 peerPort() const;

    qint64 receiveBufferSize() const;
    void setReceiveBufferSize(qint64 bufferSize);

    qint64 sendBufferSize() const;
    void setSendBufferSize(qint64 bufferSize);

    // native functions
    int option(SocketOption option) const;
    bool setOption(SocketOption option, int value);
    
    bool waitForRead(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
			    bool checkRead, bool checkWrite,
			    int msecs = 30000, bool *timedOut = 0) const;

    QAbstractSocket::SocketError error() const;
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

    QAbstractSocket::SocketType socketType;
    QAbstractSocket::NetworkLayerProtocol socketProtocol;
    QAbstractSocket::SocketState socketState;
    mutable QAbstractSocket::SocketError socketError;
    mutable QString socketErrorString;

    QHostAddress peerAddress;
    quint16 peerPort;

    QHostAddress localAddress;
    quint16 localPort;


#ifdef Q_OS_WIN
    QWindowsSockInit winSock;
#endif

    enum ErrorString {
        NonBlockingInitFailedErrorString, 
        BroadcastingInitFailedErrorString,
        NoIpV6ErrorString, 
        RemoteHostClosedErrorString,
        TimeOutErrorString,
        ResourceErrorString,
        OperationUnsupportedErrorString,
        ProtocolUnsupportedErrorString, 
        InvalidSocketErrorString, 
        UnreachableErrorString, 
        AccessErrorString, 
        ConnectionTimeOutErrorString, 
        ConnectionRefusedErrorString, 
        AddressInuseErrorString, 
        AddressNotAvailableErrorString, 
        AddressProtectedErrorString, 
        DatagramTooLargeErrorString, 
        SendDatagramErrorString,
        ReceiveDatagramErrorString,
        WriteErrorString, 
        ReadErrorString,
        PortInuseErrorString
    };

    void setError(QAbstractSocket::SocketError error, ErrorString errorString) const;

    // native functions
    int option(QSocketLayer::SocketOption option) const;
    bool setOption(QSocketLayer::SocketOption option, int value);

    bool createNewSocket(QAbstractSocket::SocketType type, QAbstractSocket::NetworkLayerProtocol protocol);

    bool nativeConnect(const QHostAddress &address, quint16 port);
    bool nativeBind(const QHostAddress &address, quint16 port);
    bool nativeListen(int backlog);
    int nativeAccept();
    qint64 nativeBytesAvailable() const;

    bool nativeHasPendingDatagrams() const;
    qint64 nativePendingDatagramSize() const;
    qint64 nativeReceiveDatagram(char *data, qint64 maxLength,
                                     QHostAddress *address, quint16 *port);
    qint64 nativeSendDatagram(const char *data, qint64 length,
                                  const QHostAddress &host, quint16 port);
    qint64 nativeRead(char *data, qint64 maxLength);
    qint64 nativeWrite(const char *data, qint64 length);
    int nativeSelect(int timeout, bool selectForRead) const;
    int nativeSelect(int timeout, bool checkRead, bool checkWrite,
		     bool *selectForRead, bool *selectForWrite) const;

    void nativeClose();

    bool fetchConnectionParameters();

    QSocketLayer *q;
};

#endif // QSOCKETLAYER_P_H
