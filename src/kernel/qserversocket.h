/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qserversocket.h#3 $
**
** Definition of QServerSocket class
**
** Created : 990221
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qsocket.h"
#endif // QT_H


class QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket( const QObject *parent=0, const char *name=0 );
    QServerSocket( int port, const QObject *parent=0, const char *name=0 );

    int		 port() const;
    void	 setPort( int port );

    virtual void newConnection( int socket );
 
protected:
    QSocketDevice *socketDevice();
    virtual bool accept( void *internet_address ) const;

protected slots:
    void	 incomingConnection( int socket );

private:
    QServerSocketPrivate *d;
    QSocket	*sd;
};


inline int QServerSocket::port() const
{
    return serverSocket->port();
}

inline QSocket *QServerSocket::socket()
{
    return serverSocket;
}


#endif // QSERVERSOCKET_H
