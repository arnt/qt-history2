/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocket.cpp#3 $
**
** Implementation of QSocket class
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

#include "qsocket.h"
#include "qlist.h"
#include "qsocketdevice.h"

// gethostbyname
#include <netdb.h>


class QSocketPrivate {
public:
    QSocketPrivate();
    void init();

    QSocket::State	state;			// connection state
    QSocket::Mode	mode;			// mode for reading
    QString		host;			// host name
    int			port;			// host port
    QSocketDevice      *socket;			// connection socket
    QSocketNotifier    *rsn, *wsn;		// socket notifiers
    QList<QByteArray>	rba, wba;		// list of read/write bufs
    int			rsize, wsize;		// read/write total buf size
    int			rindex, windex;		// read/write index
};

QSocketPrivate::QSocketPrivate()
    : state(QSocket::Idle), mode(QSocket::Binary), host(""), port(0),
      socket(0), rsn(0), wsn(0), rsize(0), wsize(0), rindex(0), windex(0)
{
    rba.setAutoDelete( TRUE );
    wba.setAutoDelete( TRUE );
}

void QSocketPrivate::init()
{
    state = QSocket::Idle;
    mode = QSocket::Binary;
    host = "";
    port = 0;
    if ( socket ) {
	delete socket;
	socket = 0;
    }
    if ( rsn ) {
	delete rsn;
	rsn = 0;
    }
    if ( wsn ) {
	delete wsn;
	wsn = 0;
    }
    rba.clear();
    wba.clear();
    rsize = wsize = 0;
    rindex = windex = 0;
}


/*!
  Creates a QSocket object in \c Idle state.

  This socket can be used to make a connection to a host using
  the connectToHost() function.
*/

QSocket::QSocket()
{
    d = new QSocketPrivate;
}


/*!
  Creates a QSocket object for an existing connection using \a socket.
*/

QSocket::QSocket( int socket )
{
    d = new QSocketPrivate;
    d->socket = new QSocketDevice( socket, QSocketDevice::Stream );
    d->socket->setNonblocking( TRUE );
    d->socket->setOption( QSocketDevice::ReuseAddress, TRUE );
    d->state = Connection;
    d->mode = Binary;
    d->rsn = new QSocketNotifier(d->socket->socket(), QSocketNotifier::Read);
    d->wsn = new QSocketNotifier(d->socket->socket(), QSocketNotifier::Write);
    connect( d->rsn, SIGNAL(activated(int)), SLOT(sn_read()) );
    d->rsn->setEnabled( TRUE );
    connect( d->wsn, SIGNAL(activated(int)), SLOT(sn_write()) );
    // Initialize the IO device flags
    open( IO_ReadWrite );
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
}


QSocket::~QSocket()
{
    if ( state() != Idle )
	close();
    delete d;
}


QSocket::State QSocket::state() const
{
    return d->state;
}


QSocket::Mode QSocket::mode() const
{
    return d->mode;
}


void QSocket::setMode( Mode mode )
{
    if ( d->mode == mode )
	return;
    d->mode = mode;
}


void QSocket::connectToHost( const QString &host, int port )
{
    if ( d->mode != Idle )
	close();
    // Re-initialize
    d->init();
    d->state = HostLookup;
    d->mode = Binary;
    d->host = host;
    d->port = port;
    // Host lookup - no async DNS yet
    struct hostent *hp;
    hp = gethostbyname( d->host );
    if ( !hp ) {
	d->state = Idle;
	return;
    }
    // Now prepare a connection
    d->state = Connecting;
    d->socket = new QSocketDevice;
    d->socket->setOption( QSocketDevice::ReuseAddress, TRUE );
    d->socket->setNonblocking( TRUE );
    QSocketAddress a( port, 0 ); // (int)*((struct in_addr *)(hp->h_addr_list[0])) );
    d->socket->connect( a );
    // Create and setup read/write socket notifiers
    // The socket write notifier will fire when the connection succeeds
    d->rsn = new QSocketNotifier(d->socket->socket(), QSocketNotifier::Read);
    d->rsn->setEnabled( TRUE );
    d->wsn = new QSocketNotifier(d->socket->socket(), QSocketNotifier::Write);
    d->wsn->setEnabled( TRUE );
    connect( d->rsn, SIGNAL(activated(int)), SLOT(sn_read()) );
    connect( d->wsn, SIGNAL(activated(int)), SLOT(sn_write()) );    
    // Initialize the IO device flags
    open( IO_ReadWrite );
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
}


QString QSocket::host() const
{
    return d->host;
}


int QSocket::port() const
{
    return d->port;
}


/*!
  Opens the socket using the specified QIODevice file mode.  This function
  is called automatically when needed and you should not call it yourself.
  \sa close().
*/

bool QSocket::open( int m )
{
    if ( isOpen() || d->socket == 0 )
	return FALSE;
    QIODevice::setMode( m & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
  Closes the socket and sets the socket identifier to -1 (invalid).
  \sa open()
*/

void QSocket::close()
{
    if ( !isOpen() )				// already closed
	return;
    if ( d->socket )
	d->socket->close();
    d->init();					// reinitialize
}


bool QSocket::skipReadBuf( int nbytes, char *copyInto )
{
    if ( nbytes <= 0 || nbytes > d->rsize )
	return FALSE;
    int copyPtr = 0;
    d->rsize -= nbytes;    
    while ( TRUE ) {
	QByteArray *a = d->rba.first();
	if ( d->rindex + nbytes >= (int)a->size() ) {
	    if ( copyInto ) {
	    }
	    nbytes -= a->size() - d->rindex;	// got rid of whole buffer
	    d->rba.remove();
	    d->rindex = 0;
	    if ( nbytes == 0 )
		return TRUE;
	} else {
	    d->rindex += nbytes;
	    return TRUE;
	}
    }
    return FALSE;
}


bool QSocket::skipWriteBuf( int nbytes )
{
    if ( nbytes <= 0 || nbytes > d->wsize )
	return FALSE;
    d->wsize -= nbytes;
    while ( TRUE ) {
	QByteArray *a = d->wba.first();
	if ( d->windex + nbytes >= (int)a->size() ) {
	    nbytes -= a->size() - d->windex;	// got rid of whole buffer
	    d->wba.remove();
	    d->windex = 0;
	    if ( nbytes == 0 )
		return TRUE;
	} else {
	    d->windex += nbytes;
	    return TRUE;
	}
    }
    return FALSE;
}



/*!
  Implementation of the abstract virtual QIODevice::flush() function.
  This implementation is a no-op.
*/

void QSocket::flush()
{
}


/*!
  Returns the number of bytes that can be read.
*/

uint QSocket::size() const
{
    return d->rsize;
}


/*!
  Returns the current read index.  Since QSocket is a sequential
  device, the current read index is always zero.
*/

int QSocket::at() const
{
    return 0;
}


/*!
  Moves the read index forward and returns TRUE if the operation
  was successful.
*/

bool QSocket::at( int index )
{
    if ( index < 0 || index >= d->rsize )
	return FALSE;
    skipReadBuf( index, 0 );			// throw away data 0..index-1
    return TRUE;
}


/*!
  Returns TRUE if there is no more data to read, otherwise FALSE.
*/

bool QSocket::atEnd() const
{
    return d->rsize == 0;
}


/*!
  Returns the number of bytes available for reading, same as size().
*/

int QSocket::bytesAvailable() const
{
    return d->rsize;
}


/*!
  Reads max \a maxlen bytes from the socket into \a data and returns
  the number of bytes read.  Returns -1 if an error occurred.
*/

int QSocket::readBlock( char *data, uint maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
#if defined(CHECK_NULL)
	warning( "QSocket::readBlock: Null pointer error" );
#endif
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QSocket::readBlock: Socket is not open" );
#endif
	return -1;
    }
    if ( (int)maxlen >= d->rsize )
	maxlen = d->rsize;
    skipReadBuf( maxlen, data );
    return maxlen;
}


/*!
  Writes \a len bytes from the socket from \a data and returns
  the number of bytes written.  Returns -1 if an error occurred.
*/

int QSocket::writeBlock( const char *data, uint len )
{
    if ( data == 0 && len != 0 ) {
#if defined(CHECK_NULL)
	warning( "QSocket::writeBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	warning( "QSocket::writeBlock: Socket is not open" );
#endif
	return -1;
    }
    if ( len == 0 )
	return 0;
    QByteArray *a = d->wba.last();
    if ( a && a->size() + len < 128 ) {		// small buffer, resize
	int i = a->size();
	a->resize( i+len );
	memcpy( a->data()+i, data, len );
    } else {					// append new buffer
	a = new QByteArray( len );
	d->wba.append( a );
    }
    d->wsize += len;
    d->wsn->setEnabled( TRUE );			// there's data to write
    return len;
}


void QSocket::sn_read()
{
    int nbytes = d->socket->bytesAvailable();
    if ( nbytes == 0 ) {			// connection closed
	d->state = Idle;
	emit closed();
    } else if ( nbytes > 0 ) {			// data to be read
	QByteArray *a = new QByteArray( nbytes );
	int nread = d->socket->readBlock( a->data(), nbytes );
	if ( nread != nbytes ) {		// unexpected
#if defined(CHECK_RANGE)
	    warning( "QSocket::sn_read: Unexpected short read" );
#endif
	    a->resize( nread );
	}
	d->rba.append( a );
	d->rsize += nread;
	emit readyRead();
    }
}


void QSocket::sn_write()
{
    if ( d->state == Connecting ) {		// connection established
	emit connected();
    } else if ( d->state == Connection ) {
	emit readyWrite();
    }
    if ( d->wsize > 0 ) {
	QByteArray *a = d->wba.first();
	int nwritten = d->socket->writeBlock( a->data() + d->windex,
					      a->size() - d->windex );
	if ( nwritten == (int)a->size() - d->windex ) {
	    d->wba.remove();
	    d->windex = 0;
	} else {
	    d->windex += nwritten;
	}
	d->wsize -= nwritten;
    }
    d->wsn->setEnabled( d->wsize > 0 );		// write if there's data
}
