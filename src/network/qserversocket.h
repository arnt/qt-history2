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

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qhostaddress.h"
#include "qsocketdevice.h" // ### remove or keep for users' convenience?
#endif // QT_H

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QServerSocketPrivate;


class QM_EXPORT_NETWORK QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket(QObject *parent = 0);
    QServerSocket(Q_UINT16 port, int backlog = 1, QObject *parent = 0);
    QServerSocket(const QHostAddress &address, Q_UINT16 port, int backlog = 1, QObject *parent=0);
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QServerSocket(QObject *parent, const char *name);
    QT_COMPAT_CONSTRUCTOR QServerSocket(Q_UINT16 port, int backlog, QObject *parent,
                                        const char *name);
    QT_COMPAT_CONSTRUCTOR QServerSocket(const QHostAddress &address, Q_UINT16 port, int backlog,
                                        QObject *parent, const char *name);
#endif
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

#endif
