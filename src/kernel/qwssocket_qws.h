/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice.h#14 $
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

#ifndef QWSSOCKETDEVICE_H
#define QWSSOCKETDEVICE_H

#ifndef QT_H
#include "qsocketdevice.h"
#include "qsocket.h"
#include "qserversocket.h"
#endif // QT_H


class QWSSocket;

class  QWSSocketDevice: public QSocketDevice
{
friend class QWSSocket;
public:
    QWSSocketDevice( Type type = Stream, bool inet=TRUE );
    QWSSocketDevice( int socket, Type type, bool inet=TRUE );
   ~QWSSocketDevice();

    bool connect();
    virtual bool connect( const QString& localfilename );
    virtual bool bind( const QString& localfilename );

private:
    QString fname;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSSocketDevice( const QWSSocketDevice & );
    QWSSocketDevice &operator=( const QWSSocketDevice & );
#endif
};


class QWSSocket : public QSocket
{
    Q_OBJECT
public:
    QWSSocket( QObject *parent=0, const char *name=0 );
   ~QWSSocket();
	        
    virtual void setSocket( int socket, bool inet=TRUE );
    virtual void connectToLocalFile( const QString &file );

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSSocket( const QWSSocket & );
    QWSSocket &operator=( const QWSSocket & );
#endif
};


class QWSServerSocket : public QServerSocket
{
    Q_OBJECT
public:
    QWSServerSocket( const QString& localfile, int backlog = 0,
		     QObject *parent=0, const char *name=0 );
   ~QWSServerSocket();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QWSServerSocket( const QWSServerSocket & );
    QWSServerSocket &operator=( const QWSServerSocket & );
#endif
};


#endif // QWSSOCKETDEVICE_H
