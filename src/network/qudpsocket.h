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

#ifndef QUDPSOCKET_H
#define QUDPSOCKET_H

#include "qabstractsocket.h"
#include "qhostaddress.h"

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QUdpSocketPrivate;

class QM_EXPORT_NETWORK QUdpSocket : public QAbstractSocket
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QUdpSocket)
public:
    QUdpSocket(QObject *parent = 0);
    virtual ~QUdpSocket();

    bool bind(const QHostAddress &address, Q_UINT16 port);
    bool bind(Q_UINT16 port = 0);

    bool hasPendingDatagrams() const;
    Q_LONGLONG pendingDatagramSize() const;
    Q_LONGLONG readDatagram(char *data, Q_LONGLONG maxlen,
                            QHostAddress *host = 0, Q_UINT16 *port = 0);
    Q_LONGLONG writeDatagram(const char *data, Q_LONGLONG len,
                             const QHostAddress &host, Q_UINT16 port);
    inline Q_LONGLONG writeDatagram(const QByteArray &datagram,
                                    const QHostAddress &host, Q_UINT16 port)
        { return writeDatagram(ba.data(), ba.size(), host, port); }

private:
    Q_DISABLE_COPY(QUdpSocket)
};

#endif // QUdpSOCKET_H
