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

#ifndef Q3SOCKETDEVICE_H
#define Q3SOCKETDEVICE_H

#ifndef QT_H
#include "qiodevice.h"
#include "qhostaddress.h" // int->QHostAddress conversion
#endif // QT_H

#ifndef QT_NO_NETWORK
class Q3SocketDevicePrivate;

class Q_COMPAT_EXPORT Q3SocketDevice: public QIODevice
{
public:
    enum Type { Stream, Datagram };
    enum Protocol { IPv4, IPv6, Unknown };

    Q3SocketDevice( Type type = Stream );
    Q3SocketDevice( Type type, Protocol protocol, int dummy );
    Q3SocketDevice( int socket, Type type );
    virtual ~Q3SocketDevice();

    bool	 isValid() const;
    Type	 type() const;
    Protocol	 protocol() const;

    int		 socket() const;
    virtual void setSocket( int socket, Type type );

    bool	 open( int mode );
    void	 close();
    bool	 flush();

    // Implementation of QIODevice abstract virtual functions
    Offset	 size() const;
    Offset	 at() const;
    bool	 at( Offset );
    bool	 atEnd() const;

    bool	 blocking() const;
    virtual void setBlocking( bool );

    bool	 addressReusable() const;
    virtual void setAddressReusable( bool );

    int		 receiveBufferSize() const;
    virtual void setReceiveBufferSize( uint );
    int		 sendBufferSize() const;
    virtual void setSendBufferSize( uint );

    virtual bool connect( const QHostAddress &, Q_UINT16 );

    virtual bool bind( const QHostAddress &, Q_UINT16 );
    virtual bool listen( int backlog );
    virtual int	 accept();

    Q_LONGLONG	 bytesAvailable() const;
    Q_LONG	 waitForMore( int msecs, bool *timeout=0 ) const;
    virtual Q_LONG  writeBlock( const char *data, Q_ULONG len,
			    const QHostAddress & host, Q_UINT16 port );

#ifdef QT_COMPAT
    inline QT_COMPAT Q_LONG writeBlock(const char *data, Q_ULONG len)
        { return Q_LONG(write(data, Q_LONGLONG(len))); }
    inline QT_COMPAT Q_LONG readBlock(char *data, Q_ULONG maxlen)
        { return Q_LONG(read(data, Q_LONGLONG(maxlen))); }
#endif

    Q_UINT16	 port() const;
    Q_UINT16	 peerPort() const;
    QHostAddress address() const;
    QHostAddress peerAddress() const;

    enum Error {
	NoError,
	AlreadyBound,
	Inaccessible,
	NoResources,
	InternalError,
	Bug = InternalError, // ### remove in 4.0?
	Impossible,
	NoFiles,
	ConnectionRefused,
	NetworkFailure,
	UnknownError
    };
    Error	 error() const;

    inline bool isSequential() const { return true; }

protected:
    void setError( Error err );
    Q_LONGLONG readData(char *data, Q_LONGLONG maxlen);
    Q_LONGLONG writeData(const char *data, Q_LONGLONG len);

private:
    int fd;
    Type t;
    Q_UINT16 p;
    QHostAddress a;
    Q_UINT16 pp;
    QHostAddress pa;
    Q3SocketDevice::Error e;
    Q3SocketDevicePrivate * d;

    enum Option { Broadcast, ReceiveBuffer, ReuseAddress, SendBuffer };

    int		 option( Option ) const;
    virtual void setOption( Option, int );

    void	 fetchConnectionParameters();
#if defined(Q_OS_WIN32)
    void	 fetchPeerConnectionParameters();
#endif

    static void  init();
    int		 createNewSocket();
    Protocol	 getProtocol() const;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3SocketDevice( const Q3SocketDevice & );
    Q3SocketDevice &operator=( const Q3SocketDevice & );
#endif
};

#endif // QT_NO_NETWORK

#endif // Q3SOCKETDEVICE_H
