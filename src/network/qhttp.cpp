#include "qhttp.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "qsocket.h"
#include "qtextstream.h"

/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

/*!
  \class QHttpHeader qhttp.h
  \brief The QHttpHeader class contains header information for HTTP.

  \module network

  fnord
*/

class QHttpHeaderPrivate
{
public:
    QMap<QString,QString> m_values;
};

QHttpHeader::QHttpHeader()
    : m_bValid( TRUE )
{
    d = new QHttpHeaderPrivate;
}

QHttpHeader::QHttpHeader( const QHttpHeader& header )
    : m_bValid( header.m_bValid )
{
    d = new QHttpHeaderPrivate;
    d->m_values = header.d->m_values;
}

QHttpHeader::QHttpHeader( const QString& str )
    : m_bValid( TRUE )
{	
    d = new QHttpHeaderPrivate;
    parse( str );
}

QHttpHeader::~QHttpHeader()
{
    delete d;
}

void QHttpHeader::parse( const QString& str )
{
    QStringList lst = QStringList::split( "\r\n", str.stripWhiteSpace(), FALSE );

    if ( lst.isEmpty() )
	return;
	
    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it )
    {
	if ( !(*it).isEmpty() )
	{
	    if ( (*it)[0].isSpace() )
	    {
		if ( lines.isEmpty() )
		    qWarning("Invalid line: '%s'", (*it).latin1() );
		else
		{
		    lines.last() += " ";
		    lines.last() += (*it).stripWhiteSpace();
		}
	    }
	    else
	    {
		lines.append( (*it) );
	    }
	}
    }
    
    int number = 0;
    it = lines.begin();
    for( ; it != lines.end(); ++it )
    {
	if ( !parseLine( *it, number++ ) )
        {
	    qWarning("Invalid line: '%s'", (*it).latin1() );
	    m_bValid = FALSE;
	    return;
	}
    }
}

QTextStream& QHttpHeader::read( QTextStream& stream )
{
    m_bValid = TRUE;

    int number = 0;
    while( 1 )
    {
	QString str = stream.readLine();
	
	// Unexpected end of input ?
	if ( str.isNull() )
        {
	    m_bValid = FALSE;
	    return stream;
	}
	
	// End of header ?
	if ( str.isEmpty() )
        {
	    return stream;
	}

	// Parse the line
    	if ( !parseLine( str, number++ ) )
        {
	    m_bValid = FALSE;
	    return stream;
	}
    }

    // Never reached
    return stream;
}

QString QHttpHeader::value( const QString& key ) const
{
    return d->m_values[ key.lower() ];
}

QStringList QHttpHeader::keys() const
{
    QStringList lst;

    QMap<QString,QString>::ConstIterator it = d->m_values.begin();
    for( ; it != d->m_values.end(); ++it )
	lst.append( *it );

    return lst;
}

bool QHttpHeader::hasKey( const QString& key ) const
{
    return d->m_values.contains( key.lower() );
}

void QHttpHeader::setValue( const QString& key, const QString& value )
{
    d->m_values[ key.lower() ] = value;
}

void QHttpHeader::removeValue( const QString& key )
{
    d->m_values.remove( key.lower() );
}

bool QHttpHeader::parseLine( const QString& line, int )
{
    int i = line.find( ":" );
    if ( i == -1 )
	return FALSE;

    d->m_values[ line.left( i ).stripWhiteSpace() ] = line.mid( i + 1 ).stripWhiteSpace();

    return TRUE;
}

QString QHttpHeader::toString() const
{
    QString ret = "";

    QMap<QString,QString>::ConstIterator it = d->m_values.begin();
    for( ; it != d->m_values.end(); ++it )
	ret += it.key() + ": " + it.data() + "\r\n";

    return ret;
}

QTextStream& QHttpHeader::write( QTextStream& stream ) const
{
    stream << toString();
    return stream;
}

uint QHttpHeader::contentLength() const
{
    return d->m_values[ "content-length" ].toUInt();
}

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

void QHttpHeader::setContentLength( int len )
{
    d->m_values[ "content-length" ] = QString::number( len );
}

void QHttpHeader::setContentType( const QString& type )
{
    d->m_values[ "content-type" ] = type;
}

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

QHttpReplyHeader::QHttpReplyHeader()
{
}

QHttpReplyHeader::QHttpReplyHeader( int code, const QString& text, int version )
    : QHttpHeader(), m_code( code ), m_text( text ), m_version( version )
{
}

QHttpReplyHeader::QHttpReplyHeader( const QHttpReplyHeader& header )
    : QHttpHeader( header ), m_code( header.m_code ), m_text( header.m_text ), m_version( header.m_version )
{
}

QHttpReplyHeader::QHttpReplyHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

void QHttpReplyHeader::setReply( int code, const QString& text, int version )
{
    m_code = code;
    m_text = text;
    m_version = version;
}

int QHttpReplyHeader::replyCode() const
{
    return m_code;
}

QString QHttpReplyHeader::replyText() const
{
    return m_text;
}

int QHttpReplyHeader::version() const
{
    return m_version;
}

bool QHttpReplyHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QString l = line.simplifyWhiteSpace();
    if ( l.length() < 10 )
	return FALSE;

    if ( l.left( 5 ) == "HTTP/" && l[5].isDigit() && l[6] == '.' && l[7].isDigit() &&
	 l[8] == ' ' && l[9].isDigit() )
    {
	m_version = 10 * ( l[5].latin1() - '0' ) + ( l[7].latin1() - '0' );
	
	int pos = l.find( ' ', 9 );
	if ( pos != -1 )
        {
	    m_text = l.mid( pos + 1 );
	    m_code = l.mid( 9, pos - 9 ).toInt();
	}
	else
        {
	    m_code = l.mid( 9 ).toInt();
	    m_text = QString::null;
	}
    }
    else
	return FALSE;

    return TRUE;
}

QString QHttpReplyHeader::toString() const
{
    QString ret( "HTTP/%1.%2 %3 %4\r\n%5\r\n" );
    return ret.arg( m_version / 10 ).arg ( m_version % 10 ).arg( m_code ).arg( m_text ).arg( QHttpHeader::toString() );
}

QTextStream& operator>>( QTextStream& stream, QHttpReplyHeader& header )
{
    return header.read( stream );
}

QTextStream& operator<<( QTextStream& stream, const QHttpReplyHeader& header )
{
    return header.write( stream );
}

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

QHttpRequestHeader::QHttpRequestHeader()
    : QHttpHeader()
{
}

QHttpRequestHeader::QHttpRequestHeader( const QString& method, const QString& path, int version )
    : QHttpHeader(), m_method( method ), m_path( path ), m_version( version )
{
}

QHttpRequestHeader::QHttpRequestHeader( const QHttpRequestHeader& header )
    : QHttpHeader( header ), m_method( header.m_method ), m_path( header.m_path ), m_version( header.m_version )
{
}

QHttpRequestHeader::QHttpRequestHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

void QHttpRequestHeader::setRequest( const QString& method, const QString& path, int version )
{
    m_method = method;
    m_path = path;
    m_version = version;
}

QString QHttpRequestHeader::method() const
{
    return m_method;
}

QString QHttpRequestHeader::path() const
{
    return m_path;
}

int QHttpRequestHeader::version()
{
    return m_version;
}

bool QHttpRequestHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QStringList lst = QStringList::split( " ", line.simplifyWhiteSpace() );
    if ( lst.count() > 0 )
    {
	m_method = lst[0];
	if ( lst.count() > 1 )
        {
	    m_path = lst[1];
	    if ( lst.count() > 2 )
            {
		QString v = lst[2];
		if ( v.length() >= 8 && v.left( 5 ) == "HTTP/" && v[5].isDigit() &&
		     v[6] == '.' && v[7].isDigit() )
	        {
		    m_version = 10 * ( v[5].latin1() - '0' ) + ( v[7].latin1() - '0' );		
		    return TRUE;
		}
	    }
	}
    }

    return FALSE;
}

QString QHttpRequestHeader::toString() const
{
    QString ret( "%1 %2 HTTP/%3.%4\r\n%5\r\n" );
    return ret.arg( m_method ).arg( m_path ).arg( m_version / 10 ).arg ( m_version % 10 ).arg( QHttpHeader::toString() );
}

QTextStream& operator>>( QTextStream& stream, QHttpRequestHeader& header )
{
    return header.read( stream );
}

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

QHttpClient::QHttpClient( QObject* parent, const char* name )
    : QObject( parent, name ), m_state( QHttpClient::Idle ), m_idleTimer( 0 ),
      m_device( 0 ), m_postDevice( 0 )
{
    m_socket = new QSocket( this );
	
    connect( m_socket, SIGNAL( connected() ), this, SLOT( connected() ) );
    connect( m_socket, SIGNAL( connectionClosed() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    connect( m_socket, SIGNAL( error( int ) ), this, SLOT( error( int ) ) );
    connect( m_socket, SIGNAL( bytesWritten( int ) ), this, SLOT( bytesWritten( int ) ) );

    m_idleTimer = startTimer( 0 );
}

QHttpClient::~QHttpClient()
{
    close();
}

void QHttpClient::close()
{
    // If no connection is open -> ignore
    if ( m_state == Closed || m_state == Idle )
	return;

    m_postDevice = 0;
    m_state = Closed;

    // Already closed ?
    if ( !m_socket->isOpen() )
    {
	m_idleTimer = startTimer( 0 );
    }
    else
    {
	// Close now.
	m_socket->close();
	
	// Did close succeed immediately ?
	if ( m_socket->state() == QSocket::Idle )
        {
	    // Prepare to emit the idle() signal.
	    m_idleTimer = startTimer( 0 );
	}
    }
}

bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header )
{
    return request( hostname, port, header, QByteArray(), 0 );
}

bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data )
{
    return request( hostname, port, header, data, data.size() );
}

bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data )
{
    return request( hostname, port, header, data, data.length() );
}

bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header,
			   const char* data, uint size )
{
    // Is it allowed to make a new request now ?
    if ( m_state != Idle && m_state != Alive )
    {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    m_state = Connecting;

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( m_socket->peerName() != hostname || m_socket->state() != QSocket::Connection )
    {
	m_socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the data
    m_buffer.duplicate( data, size );
    m_header = header;

    if ( m_buffer.size() > 0 )
	m_header.setContentLength( m_buffer.size() );

    return TRUE;
}

bool QHttpClient::request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device )
{
    if ( m_state != Idle )
    {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    m_state = Connecting;

    m_postDevice = device;
    if ( !m_postDevice || !m_postDevice->isOpen() || !m_postDevice->isReadable() )
    {
	qWarning("The device passes to QHttpClient::request must be opened for reading");
	return FALSE;
    }

    m_buffer = QByteArray();

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( m_socket->peerName() != hostname || m_socket->state() != QSocket::Connection )
    {
	m_socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the header
    m_header = header;
    m_header.setContentLength( m_postDevice->size() );

    return TRUE;
}

void QHttpClient::closed()
{
    if ( m_state == Closed )
	return;

    // If the other side closed the connection then there
    // may still be data left to read.
    if ( m_state == Reading )
    {
	while( m_socket->bytesAvailable() > 0 )
	{
	    qDebug(">>>>>>>>>>>>>>>>");
	    readyRead();
	    qDebug("<<<<<<<<<<<<<<<<");
	}
	
	// If we got no Content-Length then we know
	// now that the request has completed.
	if ( m_reply.hasAutoContentLength() )
	{
	    if ( m_device )
		emit reply( m_reply, m_device );
	    else
		emit reply( m_reply, m_buffer );
	
	    // Save memory
	    m_buffer = QByteArray();
	}
	// We got Content-Length, so did we get all bytes ?
	else
	{
	    if ( m_bytesRead != m_reply.contentLength() )
	    {
		qDebug("-----------------> REQUEST FAILED 1 <-------------------");
		emit requestFailed();
	    }
	}
    }    
    else if ( m_state == Connecting || m_state == Sending )
    {
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

    if ( m_state == Connecting || m_state == Reading || m_state == Sending )
    {	
	emit requestFailed();
    }

    close();
}

void QHttpClient::bytesWritten( int )
{
    if ( !m_postDevice )
	return;

    if ( m_socket->bytesToWrite() == 0 )
    {
	int max = QMIN( 4096, m_postDevice->size() - m_postDevice->at() );
	QByteArray arr( max );

	int n = m_postDevice->readBlock( arr.data(), max );
	if ( n != max )
        {
	    qWarning("Could not read enough bytes from the device");
	    close();
	    return;
	}
	if ( m_postDevice->atEnd() )
        {
	    // qDebug("At end");
	    m_postDevice = 0;
	}

	m_socket->writeBlock( arr.data(), max );
    }
}

void QHttpClient::readyRead()
{
    if ( m_state != Reading )
    {
	m_state = Reading;
	m_buffer = QByteArray();
	m_readHeader = TRUE;
    }

    if ( m_readHeader )
    {
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
	for( i = QMAX( 0, s - 3 ); !end && i+3 < s + n; ++i )
        {
	    if ( d[i] == '\r' && d[i+1] == '\n' && d[i+2] == '\r' && d[i+3] == '\n' )
		end = TRUE;
	}
	
	if ( end )
        {
	    --i;
	    m_readHeader = FALSE;
	    m_buffer[i] = 0;
	    m_reply = QHttpReplyHeader( QString( m_buffer ) );
	
	    // Check header
	    if ( !m_reply.isValid() )
	    {
		emit requestFailed();
		close();
		return;
	    }
	
	    // Handle data that was already read
	    uint n = QMIN( m_reply.contentLength(), m_buffer.size() - i - 4 );
	    if ( m_reply.hasAutoContentLength() )
		n = m_buffer.size() - i - 4;
	    m_bytesRead = n;
	
	    if ( m_device )
	    {
		// Write the data to file
		m_device->writeBlock( m_buffer.data() + i + 4, n );
	    }
	    else
	    {
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

    if ( !m_readHeader )
    {
	uint n = m_socket->bytesAvailable();
	if ( n > 0 )
        {
	    if ( !m_reply.hasAutoContentLength() )
		n = QMIN( m_reply.contentLength() - m_bytesRead, n );
	
	    if ( m_device )
	    {
		QByteArray arr( n );
		n = m_socket->readBlock( arr.data(), n );
		m_device->writeBlock( arr.data(), n );
	    }
	    else
	    {
		if ( m_reply.hasAutoContentLength() )
		    m_buffer.resize( m_buffer.size() + n );
		n = m_socket->readBlock( m_buffer.data() + m_bytesRead, n );
	    }
	    m_bytesRead += n;
	}
	
	// Read everything ?
	// We can only know that is the content length was given in advance.
	// Otherwise we emit the signal in @ref #closed.
	if ( !m_reply.hasAutoContentLength() && m_bytesRead == m_reply.contentLength() )
        {	
	    if ( m_device )
		emit reply( m_reply, m_device );
	    else
		emit reply( m_reply, m_buffer );
	
	    // Save memory
	    m_buffer = QByteArray();

	    // Handle Keep Alive
	    switch ( m_reply.connection() )
	    {
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

QHttpClient::State QHttpClient::state() const
{
    return m_state;
}

void QHttpClient::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == m_idleTimer )
    {
	killTimer( m_idleTimer );
	m_idleTimer = 0;
	
	if ( m_state == Alive )
	{
	    emit idle();
	}
	else if ( m_state != Idle )
        {
	    m_state = Idle;
	    emit idle();
	}
    }
    else
	QObject::timerEvent( e );
}

void QHttpClient::killIdleTimer()
{
    killTimer( m_idleTimer );
    m_idleTimer = 0;
}

void QHttpClient::setDevice( QIODevice* d )
{
    if ( d == m_device )
	return;

    if ( !d->isOpen() || !d->isWritable() )
    {
	qWarning("The socket must be opened for writing");
	return;
    }

    if ( m_device == 0 )
    {
	if ( m_state == Reading && !m_readHeader )
        {
	    d->writeBlock( m_buffer.data(), m_bytesRead );
	}
    }

    m_device = d;
}

QIODevice* QHttpClient::device()
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
  \brief The QHttpServer class provides a HTTP server.

  \module network

  fnord
*/

QHttpServer::QHttpServer( int port, QObject* parent, const char* name )
    : QServerSocket( port, 0, parent, name )
{
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

QHttpConnection::QHttpConnection( int socket, QObject* parent, const char* name )
    : QObject( parent, name ), m_bytesToWrite( 0 ), m_state( Created ), m_killTimer( 0 ),
      m_allowKeepAlive( TRUE ), m_keepAliveTimeout( 10000 )
{
    m_socket = new QSocket( this );
    m_socket->setSocket( socket );

    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( readyRead() ) );
    connect( m_socket, SIGNAL( bytesWritten(int) ), this, SLOT( bytesWritten(int) ) );
    connect( m_socket, SIGNAL( connectionClosed() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( closed() ) );
    connect( m_socket, SIGNAL( error(int) ), this, SLOT( socketError(int) ) );
}

QHttpConnection::~QHttpConnection()
{
    // qDebug("QHttpConnection::~QHttpConnection()");
}

QHttpConnection::State QHttpConnection::state() const
{
    return m_state;
}

void QHttpConnection::reply( const QHttpReplyHeader& repl, const QCString& data )
{
    reply( repl, data, data.length() );
}

void QHttpConnection::reply( const QHttpReplyHeader& repl, const QByteArray& data )
{
    reply( repl, data, data.size() );
}

void QHttpConnection::reply( const QHttpReplyHeader& repl, const char* data, uint size )
{
    if ( m_state != Waiting )
    {
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
    if ( m_header.connection() == QHttpHeader::KeepAlive && m_allowKeepAlive )
    {
	r.setConnection( QHttpHeader::KeepAlive );
	QString s( "timeout=%1" );
	r.setValue( "Keep-Alive", s.arg( m_keepAliveTimeout / 1000 ) );
    }
    else
	r.setConnection( QHttpHeader::Close );

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
    if ( m_state != Reading )
    {
	m_state = Reading;
	m_buffer = QByteArray();
	m_readHeader = TRUE;
    }

    //
    // Reading the header.
    //
    if ( m_readHeader )
    {
	int n = m_socket->bytesAvailable();
	int s = m_buffer.size();
	m_buffer.resize( s + n );
	n = m_socket->readBlock( m_buffer.data() + s, n );
	
	// Search for \r\n\r\n
	const char* d = m_buffer.data();
	int i;
	bool end = FALSE;
	for( i = QMAX( 0, s - 3 ); !end && i+3 < s + n; ++i )
        {
	    if ( d[i] == '\r' && d[i+1] == '\n' && d[i+2] == '\r' && d[i+3] == '\n' )
		end = TRUE;
	}
	
	// Found the end of the header ?
	if ( end )
        {
	    // Set a trailing zero
	    --i;
	    m_buffer[i] = 0;
		
	    // Parse the header
	    m_header = QHttpRequestHeader( QString( m_buffer ) );
	    m_bytesToRead = m_header.contentLength();
	
	    // Now he have to read the data.
	    m_readHeader = FALSE;
		
	    // Test wether header was valid.
	    if ( !m_header.isValid() )
	    {
		qWarning("Invalid header.\n==========\n%s\n===========", QString( m_buffer ).latin1() );
		close();
		return;
	    }

	    // ### Check for a maximum content length
	
	    // Copy data that was already read to the beginning of the buffer.
	    // And resize the buffer so that it can hold the entire data.
	    QByteArray tmp;
	    if ( !tmp.resize( m_header.contentLength() ) )
	    {
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
    if ( !m_readHeader )
    {
	// Still need to read bytes ?
	if ( m_bytesToRead > 0 )
        {
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
	if ( m_bytesToRead <= 0 )
        {
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

void QHttpConnection::error( int )
{
    qWarning("Error in QHttpConnection");

    // Do nothing
}

void QHttpConnection::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == m_killTimer )
    {
	ASSERT( m_state == Closed || m_state == Alive );
	delete this;
    }
    else
	QObject::timerEvent( e );
}

void QHttpConnection::allowKeepAlive( bool a )
{
    m_allowKeepAlive = a;
}

bool QHttpConnection::isKeepAliveAllowed() const
{
    return m_allowKeepAlive;
}

/*!
  Passing a value of 0 means infinite waiting.
*/
void QHttpConnection::setKeepAliveTimeout( int timeout )
{
    m_keepAliveTimeout = timeout;
}

/*!
  The default value is 10 seconds.
*/
int QHttpConnection::keepAliveTimeout() const
{
    return m_keepAliveTimeout;
}

#endif
