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

#ifndef QSOCKETLAYER_H
#define QSOCKETLAYER_H
#include <qstring.h>
#include "qabstractsocket.h"

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QHostAddress;
class QSocketLayerPrivate;

class QM_EXPORT_NETWORK QSocketLayer
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

    Q_LLONG bytesAvailable() const;

    Q_LLONG read(char *data, Q_LLONG maxlen);
    Q_LLONG write(const char *data, Q_LLONG len);

    Q_LLONG readDatagram(char *data, Q_LLONG maxlen, QHostAddress *addr = 0, Q_UINT16 *port = 0);
    Q_LLONG writeDatagram(const char *data, Q_LLONG len, const QHostAddress &addr, Q_UINT16 port);
    bool hasPendingDatagrams() const;
    Q_LLONG pendingDatagramSize() const;

    Qt::SocketState socketState() const;

    QHostAddress localAddress() const;
    Q_UINT16 localPort() const;
    QHostAddress peerAddress() const;
    Q_UINT16 peerPort() const;

    Q_LLONG receiveBufferSize() const;
    void setReceiveBufferSize(Q_LLONG bufferSize);

    Q_LLONG sendBufferSize() const;
    void setSendBufferSize(Q_LLONG bufferSize);

    bool waitForRead(int msecs = 30000, bool *timedOut = 0) const;
    bool waitForWrite(int msecs = 30000, bool *timedOut = 0) const;

    Qt::SocketError socketError() const;
    QString errorString() const;

private:
    Q_DISABLE_COPY(QSocketLayer)

    QSocketLayerPrivate *d;
};

#endif
