/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocket.h#2 $
**
** Definition of QSocket class
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

#ifndef QSOCKET_H
#define QSOCKET_H

#ifndef QT_H
#include "qiodevice.h"
#include "qsocketaddress.h"
#endif // QT_H


class QSocket : public QIODevice
{
public:
    enum Type { Stream, Datagram };

    QSocket( Type type = Stream );
    QSocket( int socket, Type type );
   ~QSocket();

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

    bool	 nonblockingMode() const;
    virtual void setNonblockingMode( bool );

    enum Option { Broadcast, Debug, DontRoute, KeepAlive, Linger,
		  OobInline, ReceiveBuffer, ReuseAddress, SendBuffer };

    int		 option( Option ) const;
    virtual void setOption( Option, int );

    bool	 connect( const QSocketAddress * );

    virtual bool bind( const QSocketAddress *addr );
    virtual bool listen( int backlog );
    virtual int	 accept( QSocketAddress *addr );

    int		 bytesAvailable() const;
    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );

#if defined(_OS_WIN32_)
    static bool	initWinSock();
#endif

private:
    Type	 sock_type;
    int		 sock_fd;
};


inline bool QSocket::isValid() const
{
    return sock_type != -1;
}

inline QSocket::Type QSocket::type() const
{
    return sock_type;
}

inline int QSocket::socket() const
{
    return sock_fd;
}


#endif // QSOCKET_H
