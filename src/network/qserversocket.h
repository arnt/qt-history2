/****************************************************************************
**
** Definition of QServerSocketClass.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qhostaddress.h"
#include "qsocketdevice.h" // ### remove or keep for users' convenience?
#endif // QT_H
#ifndef QT_NO_NETWORK

#if !defined(QT_MODULE_NETWORK) || defined(QT_LICENSE_PROFESSIONAL) || defined(QT_INTERNAL_NETWORK)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QServerSocketPrivate;


class QM_EXPORT_NETWORK QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket(Q_UINT16 port, int backlog = 1,
                   QObject *parent=0, const char *name=0);
    QServerSocket(const QHostAddress & address, Q_UINT16 port, int backlog = 1,
                   QObject *parent=0, const char *name=0);
    QServerSocket(QObject *parent=0, const char *name=0);
    virtual ~QServerSocket();

    bool ok() const;

    Q_UINT16 port() const ;

    int socket() const ;
    virtual void setSocket(int socket);

    QHostAddress address() const ;

    virtual void newConnection(int socket) = 0;

protected:
    QSocketDevice *socketDevice();

private slots:
    void incomingConnection(int socket);

private:
    QServerSocketPrivate *d;
    void init(const QHostAddress & address, Q_UINT16 port, int backlog);
};

#endif // QT_NO_NETWORK
#endif // QSERVERSOCKET_H
