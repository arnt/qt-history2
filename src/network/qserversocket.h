/****************************************************************************
** $Id$
**
** Definition of QServerSocketClass.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the network
** module and therefore may only be used if the network module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSERVERSOCKET_H
#define QSERVERSOCKET_H

#ifndef QT_H
#include "qobject.h"
#include "qsocket.h"
#endif // QT_H


class QServerSocketPrivate;


class Q_EXPORT QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket( Q_UINT16 port, int backlog = 0,
		   QObject *parent=0, const char *name=0 );
    QServerSocket( const QHostAddress & address, Q_UINT16 port, int backlog = 0,
		   QObject *parent=0, const char *name=0 );
    QServerSocket( QObject *parent=0, const char *name=0 );
    virtual ~QServerSocket();

    bool ok() const;

    Q_UINT16 port() const ;

    int socket() const ;
    virtual void setSocket( int socket );

    QHostAddress address() const ;

    virtual void newConnection( int socket ) = 0;

protected:
    QSocketDevice *socketDevice();

private slots:
    void incomingConnection( int socket );

private:
    QServerSocketPrivate *d;
    void init( const QHostAddress & address, Q_UINT16 port, int backlog );
};


#endif // QSERVERSOCKET_H
