/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocketdevice.h#6 $
**
** Definition of QSocketDevice class
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

#ifndef QSOCKETDEVICE_H
#define QSOCKETDEVICE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qstring.h"
#endif // QT_H


class Q_EXPORT QSocketAddress
{
public:
    QSocketAddress();
    QSocketAddress( int port, uint ip4Addr=0 );
    QSocketAddress( const QSocketAddress & );
   ~QSocketAddress();

    QSocketAddress & operator=( const QSocketAddress & );

    int		 port()		 const;
    uint	 ip4Addr()	 const;
    QString	 ip4AddrString() const;

    bool	 operator==( const QSocketAddress & );

protected:
    void	*data()    const { return ptr; }
    int		 length()  const { return len; }
    void	 setData( void *, int );

private:
    char	*ptr;
    int		 len;

    friend class QSocketDevice;
};


class Q_EXPORT QSocketDevice : public QIODevice
{
public:
    enum Type { Stream, Datagram };

    QSocketDevice( Type type = Stream );
    QSocketDevice( int socket, Type type );
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

    bool	 nonblocking() const;
    virtual void setNonblocking( bool );

    enum Option { Broadcast, Debug, DontRoute, KeepAlive, Linger,
		  OobInline, ReceiveBuffer, ReuseAddress, SendBuffer };

    int		 option( Option ) const;
    virtual void setOption( Option, int );

    /*
    enum Error { OK, InProgress, Failed };
    Error	 lastError() const;
    */

    bool	 connect( const QSocketAddress & );

    virtual bool bind( const QSocketAddress & );
    virtual bool listen( int backlog );
    virtual int	 accept( QSocketAddress * );

    int		 bytesAvailable() const;
    int		 readBlock( char *data, uint maxlen );
    int		 writeBlock( const char *data, uint len );

    int		 getch();
    int		 putch( int );
    int		 ungetch(int);

#if defined(_OS_WIN32_)
    static bool	initWinSock();
#endif

private:
    Type	 sock_type;
    int		 sock_fd;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSocketDevice( const QSocketDevice & );
    QSocketDevice &operator=( const QSocketDevice & );
#endif
};


inline bool QSocketDevice::isValid() const
{
    return sock_type != -1;
}

inline QSocketDevice::Type QSocketDevice::type() const
{
    return sock_type;
}

inline int QSocketDevice::socket() const
{
    return sock_fd;
}


#endif // QSOCKETDEVICE_H
