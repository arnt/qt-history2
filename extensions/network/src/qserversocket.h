/****************************************************************************
** $Id$
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
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


class QServerSocket : public QObject
{
    Q_OBJECT
public:
    QServerSocket( int port, int backlog = 0,
		   QObject *parent=0, const char *name=0 );
    QServerSocket( const QHostAddress & address, int port, int backlog = 0,
		   QObject *parent=0, const char *name=0 );
    ~QServerSocket();

    bool ok() const;

    uint port() const ;
    int socket() const ;

    QHostAddress address() const ;

    virtual void newConnection( int socket ) = 0;

protected:
    QServerSocket( QObject *parent=0, const char *name=0 );
    bool setSocketDevice( QSocketDevice *sd, int backlog = 0 );
    QSocketDevice *socketDevice();

private slots:
    void incomingConnection( int socket );

private:
    QServerSocketPrivate *d;
    void init( const QHostAddress & address, int port, int backlog );
};


#endif // QSERVERSOCKET_H
