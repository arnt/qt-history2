/****************************************************************************
** $Id: //depot/qt/main/src/network/qsocket.cpp#20 $
**
** Implementation of QSocket class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsocket.h"
#ifndef QT_NO_NETWORK
#include "qlist.h"
#include "qsocketdevice.h"
#include "qdns.h"

#include <string.h>
#include <errno.h>


//#define QSOCKET_DEBUG


// Private class for QSocket

class QSocketPrivate {
public:
    QSocketPrivate( QSocket *o );
   ~QSocketPrivate();
    void close();

    QSocket            *owner;			// the owner of the d pointer
    QSocket::State	state;			// connection state
    QString		filename;
    QString		host;			// host name
    Q_UINT16		port;			// host port
    QSocketDevice      *socket;			// connection socket
    QSocketNotifier    *rsn, *wsn;		// socket notifiers
    QList<QByteArray>	rba, wba;		// list of read/write bufs
    QHostAddress	addr;			// connection address
    int			rsize, wsize;		// read/write total buf size
    int			rindex, windex;		// read/write index
#ifndef QT_NO_DNS
    QDns	       *dns;
#endif
};

QSocketPrivate::QSocketPrivate( QSocket *o )
    : owner( o ),
      state(QSocket::Idle), host(QString::fromLatin1("")), port(0),
      socket(0), rsn(0), wsn(0), rsize(0), wsize(0), rindex(0), windex(0)
{
#ifndef QT_NO_DNS
    dns = 0;
#endif
    rba.setAutoDelete( TRUE );
    wba.setAutoDelete( TRUE );
}

QSocketPrivate::~QSocketPrivate()
{
    close();
    delete socket;
#ifndef QT_NO_DNS
    delete dns;
#endif
}

void QSocketPrivate::close()
{
    // Order is important here - the socket notifiers must go away
    // before the socket does, otherwise libc or the kernel will
    // become unhappy.
    delete rsn;
    rsn = 0;
    delete wsn;
    wsn = 0;
    socket->close();
    rsize = wsize = 0;
    rba.clear(); wba.clear();
    rindex = windex = 0;
}

/*!
  \class QSocket qsocket.h
  \brief The QSocket class provides a buffered TCP connection.

  \module network

  It provides a totally non-blocking QIODevice, and modifies and
  extends the API of QIODevice with socket-specific code.

  The functions you're likely to call most are connectToHost(),
  bytesAvailable(), canReadLine() and the ones it inherits from
  QIODevice.

  connectToHost() is the most-used function.  As its name implies, it
  opens a connection to a named host.

  Most network protocols are either packet-oriented or line-oriented.
  canReadLine() indicates whether a connection contains an entire
  unread line or not, and bytesAvailable() returns the number of bytes
  available for reading.

  The signals error(), connected(), readyRead() and connectionClosed()
  inform you of the progress of the connection.  There are also some
  less commonly used signals.  hostFound() is emitted when
  connectToHost() has finished its DSN lookup and is starting its TCP
  connection. delayedCloseFinished() is emitted when close()
  succeeds().  bytesWritten() is emitted when QSocket moves data from
  its "to be written" queue into the TCP implementation.

  There are several access functions for the socket: state() returns
  whether the object is idle, is doing a DNS lookup, is connecting,
  has an operational connection, etc.  address() and port() return the
  IP address and port used for the connection, peerAddress() and
  peerPort() return the IP address and port used by the peer, and
  peerName() returns the name of the peer (normally the name that was
  passed to connectToHost()). socket() returns a pointer to the
  QSocketDevice used for this socket.

  QSocket inherits QIODevice, and reimplements some of the functions.
  In general, you can treat it as a QIODevice for writing, and mostly
  also for reading.  The match isn't perfect, since the QIODevice API
  is designed for devices that are controlled by the same machine, and
  an asynchronous peer-to-peer network connection isn't quite like
  that.  For example, there is nothing that matches QIODevice::size()
  exactly.  The documentation for each of open(), close(), flush(),
  size(), at(), atEnd(), readBlock(), writeBlock(), getch(), putch(),
  ungetch() and readLine() describe the differences in detail.

  \sa QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
  Creates a QSocket object in \c QSocket::Idle state.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QSocket::QSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QSocketPrivate( this );
	setSocketDevice( 0 );
    setFlags( IO_Direct );
    setStatus( IO_Ok );
}


/*!
  Destructs the socket.  Closes the connection if necessary.
  \sa close()
*/

QSocket::~QSocket()
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): Destroy", name() );
#endif
    if ( state() != Idle )
	close();
    Q_ASSERT( d != 0 );
    delete d;
}


/*!
  Returns a pointer to the internal socket device.

  There is normally no need to manipulate the socket device directly
  since this class does the necessary setup for most applications.
*/

QSocketDevice *QSocket::socketDevice()
{
    return d->socket;
}

/*!
  Set the internal socket device to \a device. Setting this to NULL will return
  it to the default platform socket device, any existing connection will be
  disconnected before using this new device.

  The new device should not be connected before being attached to a
  QSocket, instead use \c connectToHost following this.
*/

void QSocket::setSocketDevice( QSocketDevice *device )
{
	if ( state() != Idle )
		close();
	if( d->socket )
		delete d->socket;

    if ( !device )
	{
		device = new QSocketDevice( QSocketDevice::Stream );
		device->setBlocking( FALSE );
		device->setAddressReusable( TRUE );
	}
	d->socket = device;
}

/*!
  \enum QSocket::State

  This enum defines the connection states:
  <ul>
  <li> \c QSocket::Idle if there is no connection
  <li> \c QSocket::HostLookup during a DNS lookup
  <li> \c QSocket::Connecting during TCP connection establishment
  <li> \c QSocket::Connected when there is an operational connection
  <li> \c QSocket::Closing if the socket is closing down, but is not
  yet closed.
  </ul>
*/

/*!
  Returns the current state of the socket connection.

  \sa QSocket::State
*/

QSocket::State QSocket::state() const
{
    return d->state;
}


#ifndef QT_NO_DNS

/*!  Attempts to make a connection to \a host on the specified \a port
  and return immediately.

  Any connection or pending connection is closed immediately, and
  QSocket goes into \c HostLookup state. When the lookup succeeds, it
  emits hostFound(), starts a TCP connection and goes into \c
  Connecting state.  Finally, when the connection succeeds, it emits
  connected() and goes into \c Connected state.  If there is an error
  at any point, it emits error().

  \sa state()
*/

void QSocket::connectToHost( const QString &host, Q_UINT16 port )
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s)::connectToHost: host %s, port %d",
	    name(), host.ascii(), port );
#endif
    setSocket( d->socket->createNewSocket());

    d->state = HostLookup;
    d->host = host;
    d->port = port;
    d->dns = new QDns( host, QDns::A );
    // try if the address is already available (for faster connecting...)
    tryConnecting();
    if ( d->state == HostLookup ) {
	connect( d->dns, SIGNAL(resultsReady()), this, SLOT(tryConnecting()) );
    }
};

#endif


/*!
  This private slots continues the connection process where connectToHost()
  leaves off.
*/

void QSocket::tryConnecting()
{
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s)::tryConnecting()", name() );
#endif
    // ### this ifdef isn't correct - addresses() also does /etc/hosts and
#ifndef QT_NO_DNS
    static QValueList<QHostAddress> l;
    if ( d->state == HostLookup ) {
	// numeric-address-as-string handling.
	l = d->dns->addresses();
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s)::tryConnecting: host %s, port %d, %d addresses",
		name(), d->host.ascii(), d->port, l.count() );
#endif
	if ( l.isEmpty() ) {
	    if ( !d->dns->isWorking() ) {
		d->state = Idle;
		emit error( ErrHostNotFound );
	    }
	    return;
	}
	emit hostFound();
	d->state = Connecting; // enter the next if clause
    }

    if ( d->state == Connecting ) {
	// ### hack: just use the first address
	if ( d->socket->connect( l[0], d->port ) == FALSE ) {
	    if ( d->socket->error() == QSocketDevice::NoError ) {
		if ( d->wsn )
		    d->wsn->setEnabled( TRUE );
		// write notifier will not fire in all cases (at least under Windows), so:
		//QTimer::singleShot( 100, this, SLOT(tryConnecting()) ); ### this does not really work
		return; // not serious, try again later
	    }
	    d->state = Idle;
	    emit error( ErrConnectionRefused );
	    return;
	}
#if defined(QSOCKET_DEBUG)
	QString canonical = d->dns->canonicalName();
	if ( !canonical.isNull() && canonical != d->host )
	    qDebug( "Connecting to %s", canonical.ascii() );
	qDebug( "QSocket (%s)::tryConnecting: Connect to IP address %s",
		name(), l[0].toString().ascii() );
#endif
	// The socket write notifier will fire when the connection succeeds
	if ( d->wsn )
	    d->wsn->setEnabled( TRUE );
    }
#endif
}

/*!
  \enum QSocket::Error

  This enum specifies the possible errors:
  <ul>
  <li> \c QSocket::ErrConnectionRefused if the connection was refused
  <li> \c QSocket::ErrHostNotFound if the host was not found
  <li> \c QSocket::ErrSocketRead if a read from the socket failed
  </ul>
*/

/*!
  \fn void QSocket::error( int )

  This signal is emitted after an error occurred.
*/

/*!
  \fn void QSocket::hostFound()

  This signal is emitted after connectToHost() has been called and the
  host lookup has succeeded.

  \sa connected()
*/


/*!
  \fn void QSocket::connected()

  This signal is emitted after connectToHost() has been called and a
  connection has been successfully established.

  \sa connectToHost(), connectionClosed()
*/


/*!
  \fn void QSocket::connectionClosed()

  This signal is emitted when the other end has closed the connection.
  The read buffers may contain buffered input data which you can read
  after the connection was closed.

  \sa connectToHost(), close()
*/


/*!
  \fn void QSocket::delayedCloseFinished()

  This signal is emitted when a delayed close is finished.

  If you call close() and there is buffered output data to be written, QSocket
  goes into the \c QSocket::Closing state and returns immediately. It will
  then keep writing to the socket until all the data has been written. Then,
  the delayCloseFinished() signal is emitted.

  \sa close()
*/


/*!
  \fn void QSocket::readyRead()

  This signal is emitted when there is incoming data to be read.

  Every time when there is new incoming data this signal is emitted once. Keep
  in mind that new incoming data is only reported once; i.e. if you do not read
  all data, this signal is not emitted again unless new data arrives on the
  socket.

  \sa readBlock(), readLine(), bytesAvailable()
*/


/*!
  \fn void QSocket::bytesWritten( int nbytes )

  This signal is emitted when data actually has been written to the
  network.  The \a nbytes parameter says how many bytes were written.

  The bytesToWrite() function is often used in the same context, and
  it tells how many buffered bytes there are left to write.

  \sa writeBlock(), bytesToWrite()
*/


/*!
  Opens the socket using the specified QIODevice file mode \a m.  This function
  is called automatically when needed and you should not call it yourself.
  \sa close()
*/

bool QSocket::open( int m )
{
    if ( isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QSocket::open: Already open" );
#endif
	return FALSE;
    }
    QIODevice::setMode( m & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
  Closes the socket.

  The mode is set to \c QSocket::Binary and the read buffer is cleared.

  If the output buffer is empty, the state is set to \c QSocket::Idle
  and the connection is terminated immediately.  If the output buffer
  still contains data to be written, QSocket goes into the
  \c QSocket::Closing state and the rest of the data will be written.
  When all of the outgoing data have been written, the state is set
  to \c QSocket::Idle and the connection is terminated.  At this
  point, the delayedCloseFinished() signal is emitted.

  \sa state(), setMode(), bytesToWrite()
*/

void QSocket::close()
{
    if ( !isOpen() || d->state == Idle )	// already closed
	return;
    if ( d->state == Closing )
	return;
    if ( !d->rsn || !d->wsn )
	return;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): close socket", name() );
#endif
    if ( d->socket && d->wsize ) {		// there's data to be written
	d->state = Closing;
	if ( d->rsn )
	    d->rsn->setEnabled( FALSE );
	if ( d->wsn )
	    d->wsn->setEnabled( TRUE );
	d->rba.clear();				// clear incoming data
	d->rindex = d->rsize = 0;
	return;
    }
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
	d->close();
    d->state = Idle;
}


/*!
  This function consumes \a nbytes bytes of data from the read buffer and
  copies it into \a sink.
*/

bool QSocket::consumeReadBuf( int nbytes, char *sink )
{
    if ( nbytes <= 0 || nbytes > d->rsize )
	return FALSE;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): consumeReadBuf %d bytes", name(), nbytes );
#endif
    d->rsize -= nbytes;
    while ( TRUE ) {
	QByteArray *a = d->rba.first();
	if ( d->rindex + nbytes >= (int)a->size() ) {
	    // Here we skip the whole byte array and get the next later
	    int len = a->size() - d->rindex;
	    if ( sink ) {
		memcpy( sink, a->data()+d->rindex, len );
		sink += len;
	    }
	    nbytes -= len;
	    d->rba.remove();
	    d->rindex = 0;
	    if ( nbytes == 0 ) {		// nothing more to skip
		break;
	    }
	} else {
	    // Here we skip only a part of the first byte array
	    if ( sink )
		memcpy( sink, a->data()+d->rindex, nbytes );
	    d->rindex += nbytes;
	    break;
	}
    }
    return TRUE;
}


/*!
  This function consumes \a nbytes bytes of data from the write buffer.  It is
  similar to consumeReadBuf() above, except that it does not copy the data
  into another buffer.
*/

bool QSocket::consumeWriteBuf( int nbytes )
{
    if ( nbytes <= 0 || nbytes > d->wsize )
	return FALSE;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): skipWriteBuf %d bytes", name(), nbytes );
#endif
    d->wsize -= nbytes;
    while ( TRUE ) {
	QByteArray *a = d->wba.first();
	if ( d->windex + nbytes >= (int)a->size() ) {
	    nbytes -= a->size() - d->windex;
	    d->wba.remove();
	    d->windex = 0;
	    if ( nbytes == 0 )
		break;
	} else {
	    d->windex += nbytes;
	    break;
	}
    }
    return TRUE;
}



/*!
  Scans for any occurrence of \n in the read buffer.
  Stores the text in the byte array \a store if it is non-null.
*/

bool QSocket::scanNewline( QByteArray *store )
{
    if ( d->rsize == 0 )
	return FALSE;
    int i = 0; // index into 'store'
    QByteArray *a = 0;
    char *p;
    int n;
    while ( TRUE ) {
	if ( !a ) {
	    a = d->rba.first();
	    if ( !a || a->size() == 0 )
		return FALSE;
	    p = a->data() + d->rindex;
	    n = a->size() - d->rindex;
	} else {
	    a = d->rba.next();
	    if ( !a || a->size() == 0 )
		return FALSE;
	    p = a->data();
	    n = a->size();
	}
	if ( store ) {
	    while ( n-- > 0 ) {
		*(store->data()+i) = *p;
		if ( ++i == (int)store->size() )
		    store->resize( store->size() < 256
				   ? 1024 : store->size()*4 );
		switch ( *p ) {
		    case '\0':
#if defined(QSOCKET_DEBUG)
			qDebug( "QSocket (%s)::scanNewline: Oops, unexpected "
				"0-terminated text read <%s>",
				name(), store->data() );
#endif
			store->resize( i );
			return FALSE;
		    case '\n':
			*(store->data()+i) = '\0';
			store->resize( i );
			return TRUE;
		}
		p++;
	    }
	} else {
	    while ( n-- > 0 ) {
		switch ( *p++ ) {
		    case '\0':
			return FALSE;
		    case '\n':
			return TRUE;
		}
	    }
	}
    }
}


/*!
  Implementation of the abstract virtual QIODevice::flush() function.
*/

void QSocket::flush()
{
    bool osBufferFull = FALSE;
    int consumed = 0;
    while( !osBufferFull && d->state >= Connecting && d->wsize > 0 ) {
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: Write data to the socket", name() );
#endif
	QByteArray *a = d->wba.first();
	int nwritten;
	int i = 0;
	if ( (int)a->size() - d->windex < 1460 ) {
	    // Concatenate many smaller blocks.  the first may be
	    // partial, but each subsequent block is copied entirely
	    // or not at all.  the sizes here are picked so that we
	    // generally won't trigger nagle's algorithm in the tcp
	    // implementation: we concatenate if we'd otherwise send
	    // less than PMTU bytes (we assume PMTU is 1460 bytes),
	    // and concatenate up to the largest payload TCP/IP can
	    // carry.  with these precautions, nagle's algorithm
	    // should apply only when really appropriate.
	    QByteArray out( 65536 );
	    int j = d->windex;
	    int s = a->size() - j;
	    while ( a && i+s < (int)out.size() ) {
		memcpy( out.data()+i, a->data()+j, s );
		j = 0;
		i += s;
		a = d->wba.next();
		s = a ? a->size() : 0;
	    }
	    nwritten = d->socket->writeBlock( out.data(), i );
	} else {
	    // Big block, write it immediately
	    i = a->size() - d->windex;
	    nwritten = d->socket->writeBlock( a->data() + d->windex, i );
	}
	if ( nwritten ) {
	    consumeWriteBuf( nwritten );
	    consumed += nwritten;
	}
	if ( nwritten < i )
	    osBufferFull = TRUE;
    }
    if ( consumed > 0 ) {
	emit bytesWritten( consumed );
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: wrote %d bytes, %d left",
		name(), consumed, d->wsize );
#endif
    }
    if ( d->state == Closing && d->wsize == 0 ) {
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): flush: Delayed close done. Terminating.",
		name() );
#endif
	setFlags( IO_Sequential );
	setStatus( IO_Ok );
	d->close();
	d->state = Idle;
	emit delayedCloseFinished();
	return;
    }
    if ( d->wsn )
	d->wsn->setEnabled( d->wsize > 0 ); // write if there's data
}


/*!  Returns the number of incoming bytes that can be read right now
  (like bytesAvailable()).
*/

uint QSocket::size() const
{
    return bytesAvailable();
}


/*!
  Returns the current read index.  Since QSocket is a sequential
  device, the current read index is always zero.
*/

int QSocket::at() const
{
    return 0;
}


/*!  \overload
  Moves the read index forward to \a index and returns TRUE if the operation
  was successful.  Moving the index forward means skipping incoming
  data.
*/

bool QSocket::at( int index )
{
    if ( index < 0 || index > d->rsize )
	return FALSE;
    consumeReadBuf( index, 0 );			// throw away data 0..index-1
    return TRUE;
}


/*!
  Returns TRUE if there is no more data to read, otherwise FALSE.
*/

bool QSocket::atEnd() const
{
    if ( d->socket == 0 )
	return TRUE;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->bytesAvailable() )	// a little slow, perhaps...
	that->sn_read();
    return that->d->rsize == 0;
}


/*!
  Returns the number of incoming bytes that can be read, i.e. the
  size of the input buffer.  Equivalent to size().

  \sa bytesToWrite()
*/

int QSocket::bytesAvailable() const
{
    if ( d->socket == 0 )
	return 0;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->bytesAvailable() ) // a little slow, perhaps...
	(void)that->sn_read();
    return that->d->rsize;
}


/*!
  Wait upto \a msecs milliseconds for more data to be available.

  If \a msecs is -1 the call will block indefinitely.
  This is a blocking call and should be avoided in event driven
  applications.

  Returns the number of bytes available.

  \sa bytesAvailable()
*/

int QSocket::waitForMore( int msecs ) const
{
    if ( d->socket == 0 )
	return 0;
    QSocket * that = (QSocket *)this;
    if ( that->d->socket->waitForMore( msecs ) > 0 )
	(void)that->sn_read();
    return that->d->rsize;
}


/*!
  Returns the number of bytes that are waiting to be written, i.e. the
  size of the output buffer.

  \sa bytesAvailable()
*/

int QSocket::bytesToWrite() const
{
    return d->wsize;
}


/*!
  Reads max \a maxlen bytes from the socket into \a data and returns
  the number of bytes read.  Returns -1 if an error occurred.
*/

int QSocket::readBlock( char *data, uint maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QSocket::readBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QSocket::readBlock: Socket is not open" );
#endif
	return -1;
    }
    if ( (int)maxlen >= d->rsize )
	maxlen = d->rsize;
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): readBlock %d bytes", name(), maxlen );
#endif
    consumeReadBuf( maxlen, data );
    return maxlen;
}


/*!
  Writes \a len bytes to the socket from \a data and returns
  the number of bytes written.  Returns -1 if an error occurred.
*/

int QSocket::writeBlock( const char *data, uint len )
{
#if defined(QT_CHECK_NULL)
    if ( data == 0 && len != 0 ) {
	qWarning( "QSocket::writeBlock: Null pointer error" );
    }
#endif
#if defined(QT_CHECK_STATE)
    if ( !isOpen() ) {
	qWarning( "QSocket::writeBlock: Socket is not open" );
	return -1;
    }
#endif
#if defined(QT_CHECK_STATE)
    if ( d->state == Closing ) {
	qWarning( "QSocket::writeBlock: Cannot write, socket is closing" );
    }
#endif
    if ( len == 0 || d->state == Closing )
	return 0;
    QByteArray *a = d->wba.last();

    // next bit is sensitive.  if we're writing really small chunks,
    // try to buffer up since system calls are expensive, and nagle's
    // algorithm is even more expensive.  but if anything even
    // remotely large is being written, try to issue a write at once.
    // so, we try a write immediately if there isn't data queued up
    // already, and the write notifier is off, and this blob of data
    // isn't ridiculously small.

    bool writeNow = ( d->wsize == 0 && len > 512 &&
		      d->wsn && d->wsn->isEnabled() == FALSE );

    if ( a && a->size() + len < 128 ) {
	// small buffer, resize
	int i = a->size();
	a->resize( i+len );
	memcpy( a->data()+i, data, len );
    } else {
	// append new buffer
	a = new QByteArray( len );
	memcpy( a->data(), data, len );
	d->wba.append( a );
    }
    d->wsize += len;
    if ( writeNow )
	flush();
    else if ( d->wsn )
	d->wsn->setEnabled( TRUE );
#if defined(QSOCKET_DEBUG)
    qDebug( "QSocket (%s): writeBlock %d bytes", name(), len );
#endif
    return len;
}


/*!
  Reads a single byte/character from the internal read buffer.
  Returns the byte/character read, or -1 if there is nothing
  to be read.

  \sa bytesAvailable(), putch()
*/

int QSocket::getch()
{
    if ( isOpen() && d->rsize > 0 ) {
	uchar c;
	consumeReadBuf( 1, (char*)&c );
	return c;
    }
    return -1;
}


/*!
  Writes the character \a ch into the output buffer.

  Returns \a ch, or -1 if some error occurred.

  \sa getch()
*/

int QSocket::putch( int ch )
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*!
  This implementation of the virtual function QIODevice::ungetch() always
  returns -1 (error) because a QSocket is a sequential device and does not
  allow any ungetch operation.
*/

int QSocket::ungetch( int )
{
    return -1;
}


/*! Returns TRUE if it's possible to read an entire line of text from
  this socket at this time, or FALSE if not.

  Note that if the peer closes the connection unexpectedly, this
  function returns FALSE. This means that loops such as this won't
  work:

  \code
    while( socket->!canReadLine() )
        ...
  \endcode

  \sa setMode(), readLine()
*/

bool QSocket::canReadLine() const
{
    if ( ((QSocket*)this)->scanNewline( 0 ) )
	return TRUE;
    return ( bytesAvailable() > 0 &&
	     ((QSocket*)this)->scanNewline( 0 ) );
}

/*!
  \reimp
  \internal
    So that it's not hidden by our other readLine().
*/
int QSocket::readLine( char *data, uint maxlen )
{
    return QIODevice::readLine(data,maxlen);
}

/*!
  Returns a line of text including a terminating newline character (\n).
  Returns "" if canReadLine() returns FALSE.

  \sa canReadLine()
*/

QString QSocket::readLine()
{
    QByteArray a(256);
    bool nl = scanNewline( &a );
    QString s;
    if ( nl ) {
	at( a.size() );				// skips the data read
	s = QString( a );
    }
    return s;
}

/*!
  Internal slot for handling socket read notifications.
*/

void QSocket::sn_read()
{
    char buf[4096];
    int  nbytes = d->socket->bytesAvailable();
    int  nread;
    QByteArray *a = 0;

    if ( state() == Connecting ) {
	if ( nbytes > 0 ) {
	    tryConnection();
	} else {
	    // nothing to do, nothing to care about
	    return;
	}
    }
    if ( state() == Idle )
	return;

    if ( nbytes <= 0 ) {			// connection closed?
	// On Windows this may happen when the connection is still open.
	// This happens when the system is heavily loaded and we have
	// read all the data on the socket before a new WSAsyncSelect
	// event is processed. A new read operation would then block.
	// This code is also useful when QSocket is used without an
	// event loop.
	nread = d->socket->readBlock( buf, sizeof(buf) );
	if ( nread == 0 ) {			// really closed
#if defined(QSOCKET_DEBUG)
	    qDebug( "QSocket (%s): sn_read: Connection closed", name() );
#endif
	    // We keep the open state in case there's unread incoming data
	    d->state = Idle;
	    if ( d->rsn )
		d->rsn->setEnabled( FALSE );
	    if ( d->wsn )
		d->wsn->setEnabled( FALSE );
	    d->socket->close();
	    d->wba.clear();			// clear write buffer
	    d->windex = d->wsize = 0;
	    emit connectionClosed();
	    return;
	} else {
	    if ( nread < 0 ) {
		if ( d->socket->error() == QSocketDevice::NoError ) {
		    // all is fine
		    return;
		}
#if defined(QT_CHECK_RANGE)
		qWarning( "QSocket::sn_read (%s): Close error", name() );
#endif
		emit error( ErrSocketRead );	// socket close error
		return;
	    }
	    a = new QByteArray( nread );
	    memcpy( a->data(), buf, nread );
	}

    } else {					// data to be read
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): sn_read: %d incoming bytes", name(), nbytes );
#endif
	if ( nbytes > (int)sizeof(buf) ) {
	    // big
	    a = new QByteArray( nbytes );
	    nread = d->socket->readBlock( a->data(), nbytes );
	} else {
	    a = 0;
	    nread = d->socket->readBlock( buf, sizeof(buf) );
	    if ( nread > 0 ) {
		// ##### could setRawData
		a = new QByteArray( nread );
		memcpy(a->data(),buf,nread);
	    }
	}
	if ( nread < 0 ) {
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSocket::sn_read: Read error" );
#endif
	    delete a;
	    emit error( ErrSocketRead );	// socket read error
	    return;
	}
	if ( nread != (int)a->size() ) {		// unexpected
#if defined(QT_CHECK_RANGE)
	    qWarning( "QSocket::sn_read: Unexpected short read" );
#endif
	    a->resize( nread );
	}
    }
    d->rba.append( a );
    d->rsize += nread;
    emit readyRead();
}


/*!
  Internal slot for handling socket write notifications.
*/

void QSocket::sn_write()
{
    if ( d->state == Connecting ) 		// connection established?
	tryConnection();
    flush();
}

void QSocket::tryConnection()
{
    if ( d->socket->connect( d->addr, d->port ) ) {
	d->state = Connected;
#if defined(QSOCKET_DEBUG)
	qDebug( "QSocket (%s): sn_write: Got connection to %s",
		name(), peerName().ascii() );
#endif
	emit connected();
    } else {
	d->state = Idle;
	emit error( ErrConnectionRefused );
	return;
    }
}


/*!
  Returns the socket number, or -1 if there is no socket at the moment.
*/

int QSocket::socket() const
{
    if ( d->socket == 0 )
	return -1;
    return d->socket->socket();
}


/*!
  Sets the socket to use \a socket and the state() to \c Connected. The socket
  should already be connected.

  This allows one to use the QSocket class as a wrapper for other socket types
  (e.g. Unix Domain Sockets under Unix).
*/

void QSocket::setSocket( int socket )
{
    if ( state() != Idle )
		close();
    d->state = Connected;

	d->socket->setSocket(socket, QSocketDevice::Stream );
    d->rsn = new QSocketNotifier( d->socket->socket(), QSocketNotifier::Read,
				  this, "read" );
    d->wsn = new QSocketNotifier( d->socket->socket(), QSocketNotifier::Write,
				  this, "write" );
    connect( d->rsn, SIGNAL(activated(int)), SLOT(sn_read()) );
    d->rsn->setEnabled( TRUE );
    connect( d->wsn, SIGNAL(activated(int)), SLOT(sn_write()) );
    d->wsn->setEnabled( FALSE );

    // Initialize the IO device flags
    setFlags( IO_Direct );
    setStatus( IO_Ok );
    open( IO_ReadWrite );

    // hm... this is not very nice.
    d->host = QString::null;
    d->port = 0;
#ifndef QT_NO_DNS
    delete d->dns;
    d->dns = 0;
#endif
}


/*!
  Returns the host port number of this socket.
*/

Q_UINT16 QSocket::port() const
{
    if ( d->socket == 0 )
	return 0;
    return d->socket->port();
}


/*!  Returns the peer's host port number, normally as specified to the
  connectToHost() function.  If none has been set, this function
  returns 0.
*/

Q_UINT16 QSocket::peerPort() const
{
    if ( d->socket == 0 )
	return 0;
    return d->socket->peerPort();
}


/*!  Returns the host address of this socket. (This is normally be the
  main IP address of the host, but can be e.g. 127.0.0.1 for
  connections to localhost.)
*/

QHostAddress QSocket::address() const
{
    if ( d->socket == 0 ) {
	QHostAddress tmp;
	return tmp;
    }
    return d->socket->address();
}


/*!
  Returns the host address as resolved from the name specified to the
  connectToHost() function.
*/

QHostAddress QSocket::peerAddress() const
{
    if ( d->socket == 0 ) {
	QHostAddress tmp;
	return tmp;
    }
    return d->socket->peerAddress();
}


/*!
  Returns the host name as specified to the connectToHost() function.
  An empty string is returned if none has been set.
*/

QString QSocket::peerName() const
{
    return d->host;
}
#endif //QT_NO_NETWORK
