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

#ifndef QSOCKETDEVICE_H
#define QSOCKETDEVICE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#include "qhostaddress.h"
#endif // QT_H


class QSocketDevicePrivate;


class  QSocketDevice: public QIODevice
{
friend class QSocket;

public:
    enum Type { Stream, Datagram };

    QSocketDevice( Type type = Stream, bool inet=TRUE );
    QSocketDevice( int socket, Type type, bool inet=TRUE );
   ~QSocketDevice();

    bool	 isValid() const;
    Type	 type() const;
    int		 socket() const;
    virtual void setSocket( int socket, Type type );

    bool	 open( int mode );
    void	 close();
    void	 flush();

    // Implementation of QIODevice abstract virtual functions
    uint	 size() const;
    int		 at() const;
    bool	 at( int );
    bool	 atEnd() const;

    bool	 blocking() const;
    virtual void setBlocking( bool );

    bool	 addressReusable() const;
    virtual void setAddressReusable( bool );

    int		 receiveBufferSize() const;
    virtual void setReceiveBufferSize( uint );
    int		 sendBufferSize() const;
    virtual void setSendBufferSize( uint );

    virtual bool connect( const QHostAddress &, uint );
    virtual bool connect( const QString& localfilename );

    virtual bool bind( const QHostAddress &, uint );
    virtual bool bind( const QString& );
    virtual bool listen( int backlog );
    virtual int	 accept();

    int		 bytesAvailable() const;
    int		 waitForMore( int msecs );
    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );
    virtual int  writeBlock( const char *data, uint len,
			    const QHostAddress & host, uint port );

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

    uint	 port() const;
    uint	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    enum Error { NoError, AlreadyBound, Inaccessible, NoResources,
		 Bug, Impossible, NoFiles, ConnectionRefused,
		 NetworkFailure, UnknownError };
    Error 	 error() const;

private:
    int fd;
    Type t;
    uint p;
    QHostAddress a;
    uint pp;
    QHostAddress pa;
    mutable QSocketDevice::Error e;
    QSocketDevicePrivate * d;

    enum Option { Broadcast, ReceiveBuffer, ReuseAddress, SendBuffer };

    int		 option( Option ) const;
    virtual void setOption( Option, int );

    void	 fetchConnectionParameters();

    static void init();

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocketDevice( const QSocketDevice & );
    QSocketDevice &operator=( const QSocketDevice & );
#endif
};


#endif // QSOCKETDEVICE_H
