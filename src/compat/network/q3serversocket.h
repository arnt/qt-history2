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

#ifndef Q3SERVERSOCKET_H
#define Q3SERVERSOCKET_H

#ifndef QT_H
#include "QtCore/qobject.h"
#include "QtNetwork/qhostaddress.h"
#include "Qt3Support/q3socketdevice.h" // ### remove or keep for users' convenience?
#endif // QT_H

class Q3ServerSocketPrivate;

class Q_COMPAT_EXPORT Q3ServerSocket : public QObject
{
    Q_OBJECT
public:
    Q3ServerSocket( Q_UINT16 port, int backlog = 1,
		   QObject *parent=0, const char *name=0 );
    Q3ServerSocket( const QHostAddress & address, Q_UINT16 port, int backlog = 1,
		   QObject *parent=0, const char *name=0 );
    Q3ServerSocket( QObject *parent=0, const char *name=0 );
    virtual ~Q3ServerSocket();

    bool ok() const;

    Q_UINT16 port() const ;

    int socket() const ;
    virtual void setSocket( int socket );

    QHostAddress address() const ;

    virtual void newConnection( int socket ) = 0;

protected:
    Q3SocketDevice *socketDevice();

private slots:
    void incomingConnection( int socket );

private:
    Q3ServerSocketPrivate *d;
    void init( const QHostAddress & address, Q_UINT16 port, int backlog );
};

#endif // Q3SERVERSOCKET_H
