#include "qhttp.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "qsocket.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qcstring.h"
#include "qbuffer.h"

/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

class QHttpHeaderPrivate
{
public:
    QMap<QString,QString> m_values;
};

/*!
  \class QHttpHeader qhttp.h
  \brief The QHttpHeader class contains header information for HTTP.

  \module network

  fnord
*/

/*!
  Constructs an empty HTTP header.
*/
QHttpHeader::QHttpHeader()
    : m_bValid( TRUE )
{
    d = new QHttpHeaderPrivate;
}

/*!
  Copy constructor.
*/
QHttpHeader::QHttpHeader( const QHttpHeader& header )
    : m_bValid( header.m_bValid )
{
    d = new QHttpHeaderPrivate;
    d->m_values = header.d->m_values;
}

/*!
  Constructs a HTTP header for \a str.
### \a atr is parsed...
*/
QHttpHeader::QHttpHeader( const QString& str )
    : m_bValid( TRUE )
{	
    d = new QHttpHeaderPrivate;
    parse( str );
}

/*!
  Destructor.
*/
QHttpHeader::~QHttpHeader()
{
    delete d;
}

/*!
  Returns TRUE if the HTTP header is valid, otherwise FALSE.

  A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
    return m_bValid;
}

/*!
  Parses the HTTP header string \a str and add the information.
*/
void QHttpHeader::parse( const QString& str )
{
    QStringList lst = QStringList::split( "\r\n", str.stripWhiteSpace(), FALSE );

    if ( lst.isEmpty() )
	return;

    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    if ( (*it)[0].isSpace() ) {
		if ( lines.isEmpty() ) {
		    qWarning("Invalid line: '%s'", (*it).latin1() );
		} else {
		    lines.last() += " ";
		    lines.last() += (*it).stripWhiteSpace();
		}
	    } else {
		lines.append( (*it) );
	    }
	}
    }

    int number = 0;
    it = lines.begin();
    for( ; it != lines.end(); ++it ) {
	if ( !parseLine( *it, number++ ) ) {
	    qWarning("Invalid line: '%s'", (*it).latin1() );
	    m_bValid = FALSE;
	    return;
	}
    }
}

/*!
  Reads a HTTP header from the text stream \a stream and returns a reference to
  the stream.
*/
QTextStream& QHttpHeader::read( QTextStream& stream )
{
    m_bValid = TRUE;

    int number = 0;
    while( 1 ) {
	QString str = stream.readLine();
	
	// Unexpected end of input ?
	if ( str.isNull() ) {
	    m_bValid = FALSE;
	    return stream;
	}
	
	// End of header ?
	if ( str.isEmpty() ) {
	    return stream;
	}

	// Parse the line
    	if ( !parseLine( str, number++ ) ) {
	    m_bValid = FALSE;
	    return stream;
	}
    }

    // Never reached
    return stream;
}

/*!
  Looks up if the HTTP header contains an entry with the key \a key. If such an
  entry exists, this function returns the value of the entry; otherwise an
  empty string is returned.
*/
QString QHttpHeader::value( const QString& key ) const
{
    return d->m_values[ key.lower() ];
}

/*!
  Looks up all keys in the HTTP header and returns a list of them.
*/
QStringList QHttpHeader::keys() const
{
    QStringList lst;

    QMap<QString,QString>::ConstIterator it = d->m_values.begin();
    for( ; it != d->m_values.end(); ++it )
	lst.append( *it );

    return lst;
}

/*!
  Returns TRUE, if the HTTP header contains an entry with the key \a key,
  otherwise FALSE.
*/
bool QHttpHeader::hasKey( const QString& key ) const
{
    return d->m_values.contains( key.lower() );
}

/*!
  Sets the value of the entry \a key to \a value.

  If an entry with the key \a key already existed, the previous value will be
  replaced. If no such entry existed, e new one will be added to HTTP header.
*/
void QHttpHeader::setValue( const QString& key, const QString& value )
{
    d->m_values[ key.lower() ] = value;
}

/*!
  Removes the entry with the key \a key from the HTTP header.
*/
void QHttpHeader::removeValue( const QString& key )
{
    d->m_values.remove( key.lower() );
}

/*!
  Parses a single HTTP header line and add it the information.
*/
bool QHttpHeader::parseLine( const QString& line, int )
{
    int i = line.find( ":" );
    if ( i == -1 )
	return FALSE;

    d->m_values[ line.left( i ).stripWhiteSpace() ] = line.mid( i + 1 ).stripWhiteSpace();

    return TRUE;
}

/*!
  Returns a string representation of the HTTP header.
*/
QString QHttpHeader::toString() const
{
    QString ret = "";

    QMap<QString,QString>::ConstIterator it = d->m_values.begin();
    for( ; it != d->m_values.end(); ++it )
	ret += it.key() + ": " + it.data() + "\r\n";

    return ret;
}

/*!
  Writes a string representation of the HTTP header to the text stream \a
  stream and returns a reference to the stream.
*/
QTextStream& QHttpHeader::write( QTextStream& stream ) const
{
    stream << toString();
    return stream;
}

/*!
  Returns the value of the special HTTP header entry \c content-length.
*/
uint QHttpHeader::contentLength() const
{
    return d->m_values[ "content-length" ].toUInt();
}

/*!
  Returns the value of the special HTTP header entry \c content-type.
*/
QString QHttpHeader::contentType() const
{
    QString type = d->m_values[ "content-type" ];
    if ( type.isEmpty() )
	return QString::null;
    
    int pos = type.find( ";" );
    if ( pos == -1 )
	return type;
    
    return type.left( pos ).stripWhiteSpace();
}

/*!
  \enum QHttpHeader::Connection

  <ul>
  <li> Close
  <li> KeepAlive
  </ul>
*/
/*!
  Returns the value of the special HTTP header entry \c connection.
*/
QHttpHeader::Connection QHttpHeader::connection() const
{
    if ( !d->m_values.contains( "connection" ) )
	return Close;

    const char* c = d->m_values[ "connection" ].latin1();

//    if ( strcasecmp( c, "close" ) == 0 ) ### correct change?
    if ( qstrcmp( c, "close" ) == 0 )
	return Close;
//    if ( strcasecmp( c, "keep-alive" ) == 0 ) ### correct change?
    if ( qstrcmp( c, "keep-alive" ) == 0 )
	return KeepAlive;

    return Close;
}

/*!
  Sets the value of the special HTTP header entry \c content-length to \a len.
*/
void QHttpHeader::setContentLength( int len )
{
    d->m_values[ "content-length" ] = QString::number( len );
}

/*!
  Sets the value of the special HTTP header entry \c content-type to \a type.
*/
void QHttpHeader::setContentType( const QString& type )
{
    d->m_values[ "content-type" ] = type;
}

/*!
  Sets the value of the special HTTP header entry \c connection to \a con.
*/
void QHttpHeader::setConnection( QHttpHeader::Connection con )
{
    switch( con )
    {
    case Close:
	d->m_values[ "connection" ] = "close";
	break;
    case KeepAlive:
	d->m_values[ "connection" ] = "Keep-Alive";
	break;
    }
}

/****************************************************
 *
 * QHttpReplyHeader
 *
 ****************************************************/

/*!
  \class QHttpReplyHeader qhttp.h
  \brief The QHttpReplyHeader class contains reply header information for HTTP.

  \module network

  fnord
*/

/*!
  Constructs an empty HTTP reply header.
*/
QHttpReplyHeader::QHttpReplyHeader()
{
    d = 0;
}

/*!
  Constructs a HTTP reply header with....#### args?
*/
QHttpReplyHeader::QHttpReplyHeader( int code, const QString& text, int version )
    : QHttpHeader(), m_code( code ), m_text( text ), m_version( version )
{
    d = 0;
}

/*!
  Copy constructor.
*/
QHttpReplyHeader::QHttpReplyHeader( const QHttpReplyHeader& header )
    : QHttpHeader( header ), m_code( header.m_code ), m_text( header.m_text ), m_version( header.m_version )
{
    d = 0;
}

/*!
  Constructs a HTTP rely header from the string \a str.
*/
QHttpReplyHeader::QHttpReplyHeader( const QString& str )
    : QHttpHeader()
{
    d = 0;
    parse( str );
}

/*!
  Don't know about this one.
  ####???
*/
void QHttpReplyHeader::setReply( int code, const QString& text, int version )
{
    m_code = code;
    m_text = text;
    m_version = version;
}

/*!
  Returns the reply code of the HTTP reply header.
*/
int QHttpReplyHeader::replyCode() const
{
    return m_code;
}

/*!
  Returns the reply text of the HTTP reply header.
*/
QString QHttpReplyHeader::replyText() const
{
    return m_text;
}

/*!
  Returns the version of the HTTP reply header.
*/
int QHttpReplyHeader::version() const
{
    return m_version;
}

/*! \reimp
*/
bool QHttpReplyHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QString l = line.simplifyWhiteSpace();
    if ( l.length() < 10 )
	return FALSE;

    if ( l.left( 5 ) == "HTTP/" && l[5].isDigit() && l[6] == '.' &&
	    l[7].isDigit() && l[8] == ' ' && l[9].isDigit() ) {
	m_version = 10 * ( l[5].latin1() - '0' ) + ( l[7].latin1() - '0' );
	
	int pos = l.find( ' ', 9 );
	if ( pos != -1 ) {
	    m_text = l.mid( pos + 1 );
	    m_code = l.mid( 9, pos - 9 ).toInt();
	} else {
	    m_code = l.mid( 9 ).toInt();
	    m_text = QString::null;
	}
    } else {
	return FALSE;
    }

    return TRUE;
}

/*! \reimp
*/
QString QHttpReplyHeader::toString() const
{
    QString ret( "HTTP/%1.%2 %3 %4\r\n%5\r\n" );
    return ret.arg( m_version / 10 ).arg ( m_version % 10 ).arg( m_code ).arg( m_text ).arg( QHttpHeader::toString() );
}

/*!
  Reads a HTTP reply header from the text stream \a stream and stores it in \a
  header and returns a refernce to the stream.
*/
QTextStream& operator>>( QTextStream& stream, QHttpReplyHeader& header )
{
    return header.read( stream );
}

/*!
  Writes the HTTP reply header \a header to the stream \a stream and returns a
  reference to the stream.
*/
QTextStream& operator<<( QTextStream& stream, const QHttpReplyHeader& header )
{
    return header.write( stream );
}

/*!
  Returns TRUE if the server did not specify a size of the reply data, This is
  only possible if the server set the connection mode to Close. Otherwise this
  function returns FALSE.
*/
bool QHttpReplyHeader::hasAutoContentLength() const
{
    if ( connection() == Close && !hasKey( "content-length" ) )
	return TRUE;

    return FALSE;
}

/****************************************************
 *
 * QHttpRequestHeader
 *
 ****************************************************/

/*!
  \class QHttpRequestHeader qhttp.h
  \brief The QHttpRequestHeader class contains request header information for
  HTTP.

  \module network

  fnord
*/

/*!
  Constructs an empty HTTP request header.
*/
QHttpRequestHeader::QHttpRequestHeader()
    : QHttpHeader()
{
    d = 0;
}

/*!
  ### args?
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& method, const QString& path, int version )
    : QHttpHeader(), m_method( method ), m_path( path ), m_version( version )
{
    d = 0;
}

/*!
  Copy constructor.
*/
QHttpRequestHeader::QHttpRequestHeader( const QHttpRequestHeader& header )
    : QHttpHeader( header ), m_method( header.m_method ), m_path( header.m_path ), m_version( header.m_version )
{
    d = 0;
}

/*!
  Constructs a HTTP request header from the string \a str.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& str )
    : QHttpHeader()
{
    d = 0;
    parse( str );
}

/*!
  ### args?
*/
void QHttpRequestHeader::setRequest( const QString& method, const QString& path, int version )
{
    m_method = method;
    m_path = path;
    m_version = version;
}

/*!
  Returns the method of the HTTP request header.
*/
QString QHttpRequestHeader::method() const
{
    return m_method;
}

/*!
  Returns the path of the HTTP request header.
*/
QString QHttpRequestHeader::path() const
{
    return m_path;
}

/*!
  Returns the version of the HTTP request header.
*/
int QHttpRequestHeader::version()
{
    return m_version;
}

/*! \reimp
*/
bool QHttpRequestHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QStringList lst = QStringList::split( " ", line.simplifyWhiteSpace() );
    if ( lst.count() > 0 ) {
	m_method = lst[0];
	if ( lst.count() > 1 ) {
	    m_path = lst[1];
	    if ( lst.count() > 2 ) {
		QString v = lst[2];
		if ( v.length() >= 8 && v.left( 5 ) == "HTTP/" &&
			v[5].isDigit() && v[6] == '.' && v[7].isDigit() ) {
		    m_version = 10 * ( v[5].latin1() - '0' ) +
			( v[7].latin1() - '0' );		
		    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

/*! \reimp
*/
QString QHttpRequestHeader::toString() const
{
    QString ret( "%1 %2 HTTP/%3.%4\r\n%5\r\n" );
    return ret.arg( m_method ).arg( m_path ).arg( m_version / 10 ).arg ( m_version % 10 ).arg( QHttpHeader::toString() );
}

/*!
  Reads a HTTP request header from the text stream \a stream and stores it in
  \a header and returns a refernce to the stream.
*/
QTextStream& operator>>( QTextStream& stream, QHttpRequestHeader& header )
{
    return header.read( stream );
}

/*!
  Writes the HTTP request header \a header to the stream \a stream and returns
  a reference to the stream.
*/
QTextStream& operator<<( QTextStream& stream, const QHttpRequestHeader& header )
{
    return header.write( stream );
}

/****************************************************
 *
 * QHttpClient
 *
 ****************************************************/

/*!
  \class QHttpClient qhttp.h
  \brief The QHttpClient class provides a HTTP client.

  \module network

  fnord
*/

/*!
  \fn void QHttpClient::reply( const QHttpReplyHeader& repl, const QByteArray& data )

  This signal is emitted when the reply is available. Do not call request()
  in response to this signal. Instead wait for idle().
 
  If this QHttpClient has a socket set, then this signal is not emitted.
*/
/*!
  \fn void QHttpClient::reply( const QHttpReplyHeader& repl, const QIODevice* device )

  This signal is emitted if the reply is available and the data was written to
  a device.
*/
/*!
  \fn void QHttpClient::replyHeader( const QHttpReplyHeader& repl )

  This signal is emitted if the HTTP header of the reply is available.
 
  It is now possible to decide wether the reply data should be read in memory
  or rather in some device by calling setDevice().
*/
/*!
  \fn void QHttpClient::requestFailed()

  This signal is emitted if a request failed.
*/
/*!
  \fn void QHttpClient::idle()

  This signal is emitted when the QHttpClient is able to start a new request.
  The QHttpClient is either in the state Idle or Alive now.
*/

/*!
  Constructs a http client.
*/
QHttpClient::QHttpClient( QObject* parent, const char* name )
    : QObject( parent, name ), m_state( QHttpClient::Idle ), m_idleTimer( 0 ),
      m_device( 0 ), m_postDevice( 0 )
{
    d = 0;
    m_socket = new QSocket( this );
	
    connect( m_socket, SIGNAL( connected() ), this, SLOT( connected() ) );
    connect( m_socket, SIGNAL( connectionClosed() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    connect( m_socket, SIGNAL( error( int ) ), this, SLOT( error( int ) ) );
    connect( m_socket, SIGNAL( bytesWritten( int ) ), this, SLOT( bytesWritten( int ) ) );

    m_idleTimer = startTimer( 0 );
}

/*!
  Destructor. If there is an open connection, this connection is closed.
*/
QHttpClient::~QHttpClient()
{
    close();
}

/*!
  Closes the connection. This will abort a running request.

  Do not call request() in response to this signal. Instead wait for idle().
*/
void QHttpClient::close()
{
    // If no connection is open -> ignore
    if ( m_state == Closed || m_state == Idle )
	return;

    m_postDevice = 0;
    m_state = Closed;

    // Already closed ?
    if ( !m_socket->isOpen() ) {
	m_idleTimer = startTimer( 0 );
    } else {
	// Close now.
	m_socket->close();
	
	// Did close succeed immediately ?
	if ( m_socket->state() == QSocket::Idle ) {
	    // Prepare to emit the idle() signal.
	    m_idleTimer = startTimer( 0 );
	}
    }
}

/*!
  Sends a GET request to the server \a hostname at port \a port. Use the \a
  header as the HTTP request header.
*/
bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header )
{
    return request( hostname, port, header, QByteArray(), 0 );
}

/*! \overload
*/
bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data )
{
    return request( hostname, port, header, data, data.size() );
}

/*! \overload
*/
bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data )
{
    return request( hostname, port, header, data, data.length() );
}

/*!
  Sends a POST request to the server \a hostname at port \a port. Use the \a
  header as the HTTP request header. \a data is a char array of size \a size;
  it is used as the content data of the POST request.

  Call this function after the client was created or after the idle() signal
  was emitted.  On other occasions the function returns FALSE to indicate that
  it can not issue an request currently.
*/
bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, const char* data, uint size )
{
    // Is it allowed to make a new request now ?
    if ( m_state != Idle && m_state != Alive ) {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    m_state = Connecting;

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( m_socket->peerName() != hostname || m_socket->state() != QSocket::Connection ) {
	m_socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the data
    m_buffer.duplicate( data, size );
    m_header = header;

    if ( m_buffer.size() > 0 )
	m_header.setContentLength( m_buffer.size() );

    return TRUE;
}

/*!
  Send a POST request to the server \a hostname at port \a port. Use the \a
  header as the HTTP request header. The content data is read from \a device
  (the device must be opened for reading).

  Call this function after the client was created or after the idle() signal
  was emitted.  On other occasions the function returns FALSE to indicate that
  it can not issue an request currently.
*/
bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device )
{
    if ( m_state != Idle ) {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    m_state = Connecting;

    m_postDevice = device;
    if ( !m_postDevice || !m_postDevice->isOpen() || !m_postDevice->isReadable() ) {
	qWarning("The device passes to QHttpClient::request must be opened for reading");
	return FALSE;
    }

    m_buffer = QByteArray();

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( m_socket->peerName() != hostname || m_socket->state() != QSocket::Connection ) {
	m_socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the header
    m_header = header;
    m_header.setContentLength( m_postDevice->size() );

    return TRUE;
}

/*!
  Some cleanup if the connection got closed.
*/
void QHttpClient::closed()
{
    if ( m_state == Closed )
	return;

    // If the other side closed the connection then there
    // may still be data left to read.
    if ( m_state == Reading ) {
	while( m_socket->bytesAvailable() > 0 ) {
	    qDebug(">>>>>>>>>>>>>>>>");
	    readyRead();
	    qDebug("<<<<<<<<<<<<<<<<");
	}
	
	// If we got no Content-Length then we know
	// now that the request has completed.
	if ( m_reply.hasAutoContentLength() ) {
	    if ( m_device )
		emit reply( m_reply, m_device );
	    else
		emit reply( m_reply, m_buffer );
	
	    // Save memory
	    m_buffer = QByteArray();
	} else {
	    // We got Content-Length, so did we get all bytes ?
	    if ( m_bytesRead != m_reply.contentLength() ) {
		qDebug("-----------------> REQUEST FAILED 1 <-------------------");
		emit requestFailed();
	    }
	}
    } else if ( m_state == Connecting || m_state == Sending ) {
	qDebug("-----------------> REQUEST FAILED 2 <-------------------");
	emit requestFailed();
    }
    
    m_postDevice = 0;
    m_state = Closed;
    m_idleTimer = startTimer( 0 );
}

void QHttpClient::connected()
{
    m_state = Sending;

    QString str = m_header.toString();

    m_socket->writeBlock( str.latin1(), str.length() );
    m_socket->writeBlock( m_buffer.data(), m_buffer.size() );
    m_socket->flush();

    // Save memory
    m_buffer = QByteArray();
}

void QHttpClient::error( int )
{
    m_postDevice = 0;

    if ( m_state == Connecting || m_state == Reading || m_state == Sending ) {	
	emit requestFailed();
    }

    close();
}

void QHttpClient::bytesWritten( int )
{
    if ( !m_postDevice )
	return;

    if ( m_socket->bytesToWrite() == 0 ) {
	int max = QMIN( 4096, m_postDevice->size() - m_postDevice->at() );
	QByteArray arr( max );

	int n = m_postDevice->readBlock( arr.data(), max );
	if ( n != max ) {
	    qWarning("Could not read enough bytes from the device");
	    close();
	    return;
	}
	if ( m_postDevice->atEnd() ) {
	    // qDebug("At end");
	    m_postDevice = 0;
	}

	m_socket->writeBlock( arr.data(), max );
    }
}

void QHttpClient::readyRead()
{
    if ( m_state != Reading ) {
	m_state = Reading;
	m_buffer = QByteArray();
	m_readHeader = TRUE;
    }

    if ( m_readHeader ) {
	int n = m_socket->bytesAvailable();
	if ( n == 0 )
	    return;
	
	int s = m_buffer.size();
	m_buffer.resize( s + n );
	n = m_socket->readBlock( m_buffer.data() + s, n );
	
	// Search for \r\n\r\n
	const char* d = m_buffer.data();
	int i;
	bool end = FALSE;
	for( i = QMAX( 0, s - 3 ); !end && i+3 < s + n; ++i ) {
	    if ( d[i] == '\r' && d[i+1] == '\n' && d[i+2] == '\r' && d[i+3] == '\n' )
		end = TRUE;
	}
	
	if ( end ) {
	    --i;
	    m_readHeader = FALSE;
	    m_buffer[i] = 0;
	    m_reply = QHttpReplyHeader( QString( m_buffer ) );
	
	    // Check header
	    if ( !m_reply.isValid() ) {
		emit requestFailed();
		close();
		return;
	    }
	
	    // Handle data that was already read
	    uint n = QMIN( m_reply.contentLength(), m_buffer.size() - i - 4 );
	    if ( m_reply.hasAutoContentLength() )
		n = m_buffer.size() - i - 4;
	    m_bytesRead = n;
	
	    if ( m_device ) {
		// Write the data to file
		m_device->writeBlock( m_buffer.data() + i + 4, n );
	    } else {
		// Copy the data to the beginning of a new buffer.
		QByteArray tmp;
		// Resize the array. Do we know the size of the data a priori ?
		if ( m_reply.hasAutoContentLength() )
		    tmp.resize( n );
		else
		    tmp.resize( m_reply.contentLength() );
		memcpy( tmp.data(), m_buffer.data() + i + 4, n );
		m_buffer = tmp;
	    }
	
	    emit replyHeader( m_reply );
	}
    }

    if ( !m_readHeader ) {
	uint n = m_socket->bytesAvailable();
	if ( n > 0 ) {
	    if ( !m_reply.hasAutoContentLength() )
		n = QMIN( m_reply.contentLength() - m_bytesRead, n );
	
	    if ( m_device ) {
		QByteArray arr( n );
		n = m_socket->readBlock( arr.data(), n );
		m_device->writeBlock( arr.data(), n );
	    } else {
		if ( m_reply.hasAutoContentLength() )
		    m_buffer.resize( m_buffer.size() + n );
		n = m_socket->readBlock( m_buffer.data() + m_bytesRead, n );
	    }
	    m_bytesRead += n;
	}
	
	// Read everything ?
	// We can only know that is the content length was given in advance.
	// Otherwise we emit the signal in @ref #closed.
	if ( !m_reply.hasAutoContentLength() && m_bytesRead == m_reply.contentLength() ) {	
	    if ( m_device )
		emit reply( m_reply, m_device );
	    else
		emit reply( m_reply, m_buffer );
	
	    // Save memory
	    m_buffer = QByteArray();

	    // Handle Keep Alive
	    switch ( m_reply.connection() ) {
	    case QHttpHeader::KeepAlive:
		m_state = Alive;
		// Start a timer, so that we emit the keep alive signal
		// "after" this method returned.
		m_idleTimer = startTimer( 0 );
		break;
	    case QHttpHeader::Close:		
		// Close the socket
		close();
		break;
	    }
	}
    }
}

/*!
  \enum QHttpClient::State

  This enum is used to specify the state the client is in. The possible values
  are:
  <ul>
  <li> Closed: The connection was just closed, but still one can not make new
       requests using this client. Better wait for Idle.
  <li> Connecting: A request was issued and the client is looking up IP
       addresses or connecting to the remote host.
  <li> Sending: The client is sending its request to the server.
  <li> Reading: The client has sent its request and is reading the servers
       response.
  <li> Alive: The connection to the host is open. It is possible to make new
       requests.
  <li> Idle: There is no open connection. It is possible to make new requests.
  </ul>
*/
/*!
  Returns the state of the HTTP client.
*/
QHttpClient::State QHttpClient::state() const
{
    return m_state;
}

/*! \reimp
*/
void QHttpClient::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == m_idleTimer ) {
	killTimer( m_idleTimer );
	m_idleTimer = 0;
	
	if ( m_state == Alive ) {
	    emit idle();
	} else if ( m_state != Idle ) {
	    m_state = Idle;
	    emit idle();
	}
    } else {
	QObject::timerEvent( e );
    }
}

void QHttpClient::killIdleTimer()
{
    killTimer( m_idleTimer );
    m_idleTimer = 0;
}

/*!
  Sets the device of the HTTP client to \a d.

  If a device is set, then all data read by the QHttpClient - but not the HTTP
  headers - are written to this device.

  The device must be opened for writing.

  Setting the device to 0 means that subsequently read data will be read into
  memory. By default QHttpClient reads into memory.

  Setting a device makes sense when downloading very big chunks of data.
*/
void QHttpClient::setDevice( QIODevice* d )
{
    if ( d == m_device )
	return;

    if ( !d->isOpen() || !d->isWritable() ) {
	qWarning("The socket must be opened for writing");
	return;
    }

    if ( m_device == 0 ) {
	if ( m_state == Reading && !m_readHeader ) {
	    d->writeBlock( m_buffer.data(), m_bytesRead );
	}
    }

    m_device = d;
}

/*!
  Returns the device of the HTTP client.
*/
QIODevice* QHttpClient::device() const
{
    return m_device;
}

/****************************************************
 *
 * QHttpServer
 *
 ****************************************************/

/*!
  \class QHttpServer qhttp.h
  \brief The QHttpServer class provides a class that accepts connections.

  \module network

  You must overload the newConnection() function.

  Usually your implementation will create a derived object of QHttpConnection
  to handle the connection.
*/

/*!
  Constructor.
*/
QHttpServer::QHttpServer( int port, QObject* parent, const char* name )
    : QServerSocket( port, 0, parent, name )
{
    d = 0;
}

/****************************************************
 *
 * QHttpConnection
 *
 ****************************************************/

/*!
  \class QHttpConnection qhttp.h
  \brief The QHttpConnection class provides ???

  \module network

  fnord
*/

/*!
  \fn void QHttpConnection::replyFinished()

  This signal is emitted when the server has sent the entire reply succesfully.
*/
/*!
  \fn void QHttpConnection::replyFailed()

  This signal is emitted when the server has failed to sent the entire reply.
*/

/*!
  Constructor. ### args?
*/
QHttpConnection::QHttpConnection( int socket, QObject* parent, const char* name )
    : QObject( parent, name ), m_bytesToWrite( 0 ), m_state( Created ), m_killTimer( 0 ),
      m_allowKeepAlive( TRUE ), m_keepAliveTimeout( 10000 )
{
    d = 0;
    m_socket = new QSocket( this );
    m_socket->setSocket( socket );

    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    connect( m_socket, SIGNAL( bytesWritten(int) ), this, SLOT( bytesWritten(int) ) );
    connect( m_socket, SIGNAL( connectionClosed() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( error(int) ), this, SLOT( socketError(int) ) );
}

/*!
  Destructor.
*/
QHttpConnection::~QHttpConnection()
{
    // qDebug("QHttpConnection::~QHttpConnection()");
}

/*!
  \enum QHttpConnection::State

  <ul>
  <li> Created
  <li> Reading
  <li> Waiting
  <li> Writing
  <li> Alive
  <li> Closed
  </ul>
*/
/*!
  Returns the state of the connection.
*/
QHttpConnection::State QHttpConnection::state() const
{
    return m_state;
}

/*! \overload
*/
void QHttpConnection::reply( const QHttpReplyHeader& repl, const QCString& data )
{
    reply( repl, data, data.length() );
}

/*! \overload
*/
void QHttpConnection::reply( const QHttpReplyHeader& repl, const QByteArray& data )
{
    reply( repl, data, data.size() );
}

/*!
  Sends a HTTP reply with the header \a repl. \a data is a char array with the
  size \a size. This is used as the data for the HTTP reply.
*/
void QHttpConnection::reply( const QHttpReplyHeader& repl, const char* data, uint size )
{
    if ( m_state != Waiting ) {
	qWarning("QHttpConnection did not expect a call to QHttpConnection::reply." );
	
	emit replyFailed();
	
	return;
    }

    m_state = Writing;

    QHttpReplyHeader r = repl;
	
    // Fix the header if needed
    if ( size != repl.contentLength() )
	r.setContentLength( size );

    // Insert information about the connection.
    if ( m_header.connection() == QHttpHeader::KeepAlive && m_allowKeepAlive ) {
	r.setConnection( QHttpHeader::KeepAlive );
	QString s( "timeout=%1" );
	r.setValue( "Keep-Alive", s.arg( m_keepAliveTimeout / 1000 ) );
    } else {
	r.setConnection( QHttpHeader::Close );
    }

    QString str = r.toString();
	
    // Remember how many bytes we send on the wire
    m_bytesToWrite = r.contentLength() + str.length();

    m_socket->writeBlock( str.latin1(), str.length() );
    m_socket->writeBlock( data, size );
    // HACK ?
    // m_socket->flush();
}

void QHttpConnection::readyRead()
{
    ASSERT( m_state == Created || m_state == Reading || m_state == Alive );

    // Stop the timeout if this is a keep alive connection.
    if ( m_killTimer )
	killTimer( m_killTimer );

    // Start reading? Do some initialization
    if ( m_state != Reading ) {
	m_state = Reading;
	m_buffer = QByteArray();
	m_readHeader = TRUE;
    }

    //
    // Reading the header.
    //
    if ( m_readHeader ) {
	int n = m_socket->bytesAvailable();
	int s = m_buffer.size();
	m_buffer.resize( s + n );
	n = m_socket->readBlock( m_buffer.data() + s, n );
	
	// Search for \r\n\r\n
	const char* d = m_buffer.data();
	int i;
	bool end = FALSE;
	for( i = QMAX( 0, s - 3 ); !end && i+3 < s + n; ++i ) {
	    if ( d[i] == '\r' && d[i+1] == '\n' && d[i+2] == '\r' && d[i+3] == '\n' )
		end = TRUE;
	}
	
	// Found the end of the header ?
	if ( end ) {
	    // Set a trailing zero
	    --i;
	    m_buffer[i] = 0;
		
	    // Parse the header
	    m_header = QHttpRequestHeader( QString( m_buffer ) );
	    m_bytesToRead = m_header.contentLength();
	
	    // Now he have to read the data.
	    m_readHeader = FALSE;
		
	    // Test wether header was valid.
	    if ( !m_header.isValid() ) {
		qWarning("Invalid header.\n==========\n%s\n===========", QString( m_buffer ).latin1() );
		close();
		return;
	    }

	    // ### Check for a maximum content length
	
	    // Copy data that was already read to the beginning of the buffer.
	    // And resize the buffer so that it can hold the entire data.
	    QByteArray tmp;
	    if ( !tmp.resize( m_header.contentLength() ) ) {
		qWarning("Could not allocate memory");
		close();
		return;
	    }
	    int n = QMIN( tmp.size(), m_buffer.size() - i - 4 );
	    memcpy( tmp.data(), m_buffer.data() + i + 4, n );
	    m_buffer = tmp;
	
	    m_bytesToRead -= n;
	}
    }

    //
    // Reading "data"
    //
    if ( !m_readHeader ) {
	// Still need to read bytes ?
	if ( m_bytesToRead > 0 ) {
	    int n = m_socket->bytesAvailable();
	    // Are bytes available ?
	    if ( n > 0 )
            {
		n = QMIN( m_bytesToRead, n );
		n = m_socket->readBlock( m_buffer.data() + m_buffer.size() - m_bytesToRead, n );
		m_bytesToRead -= n;
	    }
	}
	
	// Did we read the entire request ?
	if ( m_bytesToRead <= 0 ) {
	    // We are waiting for the reply.
	    m_state = Waiting;
	
	    // Tell the world about the request.
	    request( m_header, m_buffer );
	
	    // Dont waste memory
	    m_buffer = QByteArray();
	}
    }
}

void QHttpConnection::bytesWritten( int n )
{
    m_bytesToWrite -= n;

    if ( m_bytesToWrite <= 0 )
    {
	if ( m_allowKeepAlive && m_header.connection() == QHttpHeader::KeepAlive )
        {
	    m_state = Alive;
	
	    emit replyFinished();
		
	    if ( m_keepAliveTimeout )
		m_killTimer = startTimer( m_keepAliveTimeout );
	}
	else
        {
	    emit replyFinished();

	    close();
	}
    }
}

/*!
  Closes the socket and starts the kill timer.  That means the socket will self
  destruct itself once the application comes back to its event loop.
*/
void QHttpConnection::close()
{
    if ( m_state == Closed )
	return;

    m_state = Closed;
	
    if ( !m_socket->isOpen() )
    {
	m_killTimer = startTimer( 0 );
    }
    else
    {
	m_socket->close();
	
	// Did close succeed immediately ?
	if ( m_socket->state() == QSocket::Idle )
        {
	    // Prepare to die.
	    m_killTimer = startTimer( 0 );
	}
    }
}

void QHttpConnection::closed()
{
    m_state = Closed;
	
    // Closed before all bytes were written ?
    if ( m_bytesToWrite > 0 )
	emit replyFailed();

    m_killTimer = startTimer( 0 );
}

void QHttpConnection::socketError( int e )
{
    error( e );

    emit replyFailed();
	
    close();
}

/*!
  \fn void QHttpConnection::request( const QHttpRequestHeader& header, const QByteArray& data )

  Grmpf
### fnord
*/

/*!
  This function is Called when an error occures. The default implementation
  does nothing. Overload this method to implement your own error handling.
*/
void QHttpConnection::error( int )
{
    qWarning("Error in QHttpConnection");

    // Do nothing
}

/*! \reimp
*/
void QHttpConnection::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == m_killTimer ) {
	ASSERT( m_state == Closed || m_state == Alive );
	delete this;
    } else {
	QObject::timerEvent( e );
    }
}

/*!
  If \a a is TRUE, the connection will try to keep it alive. If \a a is FALSE,
  the connection is not allowed to be kept alive.
*/
void QHttpConnection::allowKeepAlive( bool a )
{
    m_allowKeepAlive = a;
}

/*!
  Returns TRUE, if the connection will try to keep it alive, otherwise FALSE.
*/
bool QHttpConnection::isKeepAliveAllowed() const
{
    return m_allowKeepAlive;
}

/*!
  Sets the timeout after which a connection that is kept alive will be
  terminated. Passing a value of 0 means infinite waiting.
*/
void QHttpConnection::setKeepAliveTimeout( int timeout )
{
    m_keepAliveTimeout = timeout;
}

/*!
  Returns the timeout after which a connection that is kept alive will be
  terminated. The default value is 10 seconds.
*/
int QHttpConnection::keepAliveTimeout() const
{
    return m_keepAliveTimeout;
}

#endif
