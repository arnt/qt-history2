/****************************************************************************
** $Id$
**
** Implementation of QHttp and related classes.
**
** Created : 970521
**
** Copyright (C) 1997-2000 Trolltech AS.  All rights reserved.
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

#include "qhttp.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "qsocket.h"
#include "qtextstream.h"
#include "qmap.h"
#include "qstring.h"
#include "qstringlist.h"
#include "qcstring.h"
#include "qbuffer.h"
#include "qurloperator.h"



/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

/*
    \class QHttpHeader qhttp.h
    \brief The QHttpHeader class contains header information for HTTP.

    \ingroup io
    \module network

    In most cases you should use the more specialized derivatives of
    this class, QHttpReplyHeader and QHttpRequestHeader, rather than
    directly using QHttpHeader.

    QHttpHeader provides the header fields. A HTTP header field
    consists (according to RFC 1945) of a name followed immediately by
    a colon, a single space, and the field value. Field names are
    case-insensitive. A typical header field looks like this:
    \code
    content-type: text/html
    \endcode

    In the API the header field name is called the "key" and the
    content is called the "value". You can get and set a header field's value
    by using its key with value() and setValue(), e.g.
    \code
    header.setValue( "content-type", "text/html" );
    QString contentType = header.value( "content-type" );
    \endcode

    Some fields are so common that getters and setters are provided
    for them as a convenient alternative to using value() and
    setValue(), e.g. contentLength(), contentType(), connection(),
    setContentLength(), setContentType() and setConnection().

    Each header key has a \e single value associated with it. If you
    set the value for a key which already exists the previous value
    will be discarded.

    \sa QHttpRequestHeader QHttpReplyHeader
*/

/*!
    Constructs an empty HTTP header.
*/
QHttpHeader::QHttpHeader()
    : m_bValid( TRUE )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpHeader::QHttpHeader( const QHttpHeader& header )
    : m_bValid( header.m_bValid )
{
    m_values = header.m_values;
}

/*!
    Constructs a HTTP header for \a str.

    This constructor parses the string \a str for header fields and
    adds this information.

    \sa parse()
*/
QHttpHeader::QHttpHeader( const QString& str )
    : m_bValid( TRUE )
{
    parse( str );
}

/*!
    Destructor.
*/
QHttpHeader::~QHttpHeader()
{
}

/*!
    Assigns \a h and returns a reference to this http header.
*/
QHttpHeader& QHttpHeader::operator=( const QHttpHeader& h )
{
    m_values = h.m_values;
    m_bValid = h.m_bValid;
    return *this;
}

/*!
    Returns TRUE if the HTTP header is valid; otherwise returns FALSE.

    A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
    return m_bValid;
}

/*!
    Parses the HTTP header string \a str for header fields and adds
    the keys/values if finds. If the string is not parsed successfully
    the QHttpHeader becomes \link isValid() invalid\endlink.

    \sa toString() read()
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
    Reads a HTTP header from the text stream \a stream and returns a
    reference to the stream.

    \sa write() toString() parse()
*/
QTextStream& QHttpHeader::read( QTextStream& stream )
{
    m_bValid = TRUE;

    int number = 0;
    for( ;; ) {
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
}

/*!
    Returns the value for the entry with the given \a key. If no entry
    has this \a key, an empty string is returned.

    \sa setValue() removeValue() hasKey() keys()
*/
QString QHttpHeader::value( const QString& key ) const
{
    return m_values[ key.lower() ];
}

/*!
    Returns a list of the keys in the HTTP header.

    \sa hasKey()
*/
QStringList QHttpHeader::keys() const
{
    QStringList lst;

    QMap<QString,QString>::ConstIterator it = m_values.begin();
    for( ; it != m_values.end(); ++it )
	lst.append( *it );

    return lst;
}

/*!
    Returns TRUE, if the HTTP header has an entry with the given \a
    key; otherwise returns FALSE.

    \sa value() setValue() keys()
*/
bool QHttpHeader::hasKey( const QString& key ) const
{
    return m_values.contains( key.lower() );
}

/*!
    Sets the value of the entry with the \a key to \a value.

    If no entry with \a key exists, a new entry with the given \a key
    and \a value is created. If an entry with the \a key already
    exists, its value is discarded and replaced with the given \a
    value.

    \sa value() hasKey() removeValue()
*/
void QHttpHeader::setValue( const QString& key, const QString& value )
{
    m_values[ key.lower() ] = value;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue( const QString& key )
{
    m_values.remove( key.lower() );
}

/*!
    Parses the single HTTP header line \a line which has the format
    key, colon, space, value, and adds key/value to the headers. The
    linenumber is \a number. Returns TRUE if the line was successfully
    parsed and the key/value added; otherwise returns FALSE.

    \sa parse()
*/
bool QHttpHeader::parseLine( const QString& line, int )
{
    int i = line.find( ":" );
    if ( i == -1 )
	return FALSE;

    m_values.insert( line.left( i ).stripWhiteSpace().lower(), line.mid( i + 1 ).stripWhiteSpace() );

    return TRUE;
}

/*!
    Returns a string representation of the HTTP header.

    \sa write()
*/
QString QHttpHeader::toString() const
{
    QString ret = "";

    QMap<QString,QString>::ConstIterator it = m_values.begin();
    for( ; it != m_values.end(); ++it )
	ret += it.key() + ": " + it.data() + "\r\n";

    return ret;
}

/*!
    Writes a string representation of the HTTP header to the text
    stream \a stream and returns a reference to the stream.

    \sa read() toString()
*/
QTextStream& QHttpHeader::write( QTextStream& stream ) const
{
    stream << toString();
    return stream;
}

/*!
  Returns TRUE if the header has an entry for the special HTTP header field \c
  content-length; otherwise returns FALSE.

  \sa contentLength() setContentLength()
*/
bool QHttpHeader::hasContentLength() const
{
    return hasKey( "content-length" );
}

/*!
  Returns the value of the special HTTP header field \c content-length.

  \sa setContentLength() hasContentLength()
*/
uint QHttpHeader::contentLength() const
{
    return m_values[ "content-length" ].toUInt();
}

/*!
  Sets the value of the special HTTP header field \c content-length to \a len.

  \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength( int len )
{
    m_values[ "content-length" ] = QString::number( len );
}

/*!
  Returns TRUE if the head has an entry for the the special HTTP header field
  \c content-type; otherwise returns FALSE.

  \sa contentType() setContentType()
*/
bool QHttpHeader::hasContentType() const
{
    return hasKey( "content-type" );
}

/*!
  Returns the value of the special HTTP header field \c content-type.

  \sa setContentType() hasContentType()
*/
QString QHttpHeader::contentType() const
{
    QString type = m_values[ "content-type" ];
    if ( type.isEmpty() )
	return QString::null;

    int pos = type.find( ";" );
    if ( pos == -1 )
	return type;

    return type.left( pos ).stripWhiteSpace();
}

/*!
  Sets the value of the special HTTP header field \c content-type to \a type.

  \sa contentType() hasContentType()
*/
void QHttpHeader::setContentType( const QString& type )
{
    m_values[ "content-type" ] = type;
}

/****************************************************
 *
 * QHttpResponseHeader
 *
 ****************************************************/

/*!
    \class QHttpResponseHeader qhttp.h

    \brief The QHttpResponseHeader class contains response header information for HTTP.

    \ingroup io
    \module network

    This class is used in the QHttpClient class to report the header
    information that the client received from the server.

    This class is also used in the QHttpConnection class to send HTTP
    replies from a server to a client.

    HTTP responses have a status code that indicates the status of the
    response. This code is a 3-digit integer result code (for details
    please refer to RFC 1945). In addition to the status code, you can
    also specify a human-readable text that describes the reason for
    the code ("reason phrase"). This class allows you to set and get
    the status code and the reason phrase.

    The code, reason and protocol version can be set in the
    constructor or later with setReply(). The values can be obtained
    from statusCode(), reasonPhrase() and version().

    This class is a QHttpHeader subclass so that class's functions,
    e.g. \link QHttpHeader::setValue() setValue()\endlink, \link
    QHttpHeader::value() value()\endlink, etc. are also available.

    \sa QHttpRequestHeader QHttpClient
*/

/*!
  Constructs an empty HTTP response header.
*/
QHttpResponseHeader::QHttpResponseHeader()
{
}

/*!
  Constructs a HTTP response header with the status code \a code, the reason
  phrase \a text and the protocol-version \a version.
*/
QHttpResponseHeader::QHttpResponseHeader( int code, const QString& text, int version )
    : QHttpHeader(), m_code( code ), m_text( text ), m_version( version )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpResponseHeader::QHttpResponseHeader( const QHttpResponseHeader& header )
    : QHttpHeader( header ), m_code( header.m_code ), m_text( header.m_text ), m_version( header.m_version )
{
}

/*!
  Constructs a HTTP response header from the string \a str. The string is parsed
  and the information is set.
*/
QHttpResponseHeader::QHttpResponseHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

/*!
    Sets the status code to \a code, the reason phrase to \a text and
    the protocol-version to \a version.

  \sa statusCode() reasonPhrase() version()
*/
void QHttpResponseHeader::setStatusLine( int code, const QString& text, int version )
{
    m_code = code;
    m_text = text;
    m_version = version;
}

/*!
  Returns the status code of the HTTP response header.

  \sa setStatusLine() reasonPhrase() version()
*/
int QHttpResponseHeader::statusCode() const
{
    return m_code;
}

/*!
  Returns the reason phrase of the HTTP response header.

  \sa setStatusLine() statusCode() version()
*/
QString QHttpResponseHeader::reasonPhrase() const
{
    return m_text;
}

/*!
  Returns the protocol-version of the HTTP response header.

  \sa setStatusLine() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::version() const
{
    return m_version;
}

/*! \reimp
*/
bool QHttpResponseHeader::parseLine( const QString& line, int number )
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
QString QHttpResponseHeader::toString() const
{
    QString ret( "HTTP/%1.%2 %3 %4\r\n%5\r\n" );
    return ret.arg( m_version / 10 ).arg ( m_version % 10 ).arg( m_code ).arg( m_text ).arg( QHttpHeader::toString() );
}

/****************************************************
 *
 * QHttpRequestHeader
 *
 ****************************************************/

/*
    \class QHttpRequestHeader qhttp.h
    \brief The QHttpRequestHeader class contains request header information for
    HTTP.

    \ingroup io
    \module network

    This class is used in the QHttpClient class to report the header
    information if the client requests something from the server.

    This class is also used in the QHttpConnection class to receive
    HTTP requests by a server.

    HTTP requests have a method which describes the request's action.
    The most common requests are "GET" and "POST". In addition to the
    request method the header also includes a request-URI to specify
    the location for the method to use.

    The method, request-URI and protocol-version can be set using a
    constructor or later using setRequest(). The values can be
    obtained using method(), path() and version().

    This class is a QHttpHeader subclass so that class's functions,
    e.g. \link QHttpHeader::setValue() setValue()\endlink, \link
    QHttpHeader::value() value()\endlink, etc. are also available.


    \sa QHttpResponseHeader QHttpClient
*/

/*!
    Constructs an empty HTTP request header.
*/
QHttpRequestHeader::QHttpRequestHeader()
    : QHttpHeader()
{
}

/*!
    Constructs a HTTP request header for the method \a method, the
    request-URI \a path and the protocol-version \a version.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& method, const QString& path, int version )
    : QHttpHeader(), m_method( method ), m_path( path ), m_version( version )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpRequestHeader::QHttpRequestHeader( const QHttpRequestHeader& header )
    : QHttpHeader( header ), m_method( header.m_method ), m_path( header.m_path ), m_version( header.m_version )
{
}

/*!
    Constructs a HTTP request header from the string \a str.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& str )
    : QHttpHeader()
{
    parse( str );
}

/*!
    This function sets the request method to \a method, the
    request-URI to \a path and the protocol-version to \a version.

    \sa method() path() version()
*/
void QHttpRequestHeader::setRequest( const QString& method, const QString& path, int version )
{
    m_method = method;
    m_path = path;
    m_version = version;
}

/*!
    Returns the method of the HTTP request header.

    \sa path() version() setRequest()
*/
QString QHttpRequestHeader::method() const
{
    return m_method;
}

/*!
    Returns the request-URI of the HTTP request header.

    \sa method() version() setRequest()
*/
QString QHttpRequestHeader::path() const
{
    return m_path;
}

/*!
    Returns the protocol-version of the HTTP request header.

    \sa method() path() setRequest()
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
    QString first( "%1 %2");
    QString last(" HTTP/%3.%4\r\n%5\r\n" );
    return first.arg( m_method ).arg( m_path ) +
	last.arg( m_version / 10 ).arg( m_version % 10 ).arg( QHttpHeader::toString());
}

#if 0
/*!
    Reads a HTTP request header from the text stream \a stream and
    stores it in \a header and returns a refernce to the stream.
*/
QTextStream& operator>>( QTextStream& stream, QHttpRequestHeader& header )
{
    return header.read( stream );
}

/*!
    Writes the HTTP request header \a header to the stream \a stream
    and returns a reference to the stream.
*/
QTextStream& operator<<( QTextStream& stream, const QHttpRequestHeader& header )
{
    return header.write( stream );
}
#endif

/****************************************************
 *
 * QHttpClient
 *
 ****************************************************/

/*
    \class QHttpClient qhttp.h
    \brief The QHttpClient class provides the client side of HTTP.

    \ingroup io
    \module network

    This class provides the functionality required to send HTTP
    requests to an HTTP server and to receive replies from the server.
    It provides full control over the request header and full access
    to the reply header.

    Note that the QHttp class provides a much easier API for fetching
    single URIs (it provides the QUrlOperator interface for HTTP) and
    is more suitable for simple requirements.

    To make an HTTP request you must set up suitable HTTP headers. The
    following example demonstrates, how to request the main HTML page
    from the Trolltech home page (i.e. the URL
    http://www.trolltech.com/index.html):
    \code
    QHttpClient client;
    QHttpRequestHeader header( "GET", "/index.html" );
    client.request( "www.trolltech.com", 80, header );
    \endcode
    \omit WE SHOULD SHOW A CONNECTION\endomit

    This only makes the request. If a part of the reply arrived, the
    signal replyChunk() is emitted. After the last chunk is reported,
    the finishedSuccess() signal is emitted. This allows you to process the
    document as chunks are received, without having to wait for the
    complete transmission to be finished.

    If you need to receive the complete document before continuing,
    you might prefer to connect to the reply() signal which is emitted
    when the entire document is received.
*/

/*!
  \fn void QHttpClient::response( const QHttpResponseHeader& repl, const QByteArray& data )

  This signal is emitted when the response is available. The response header is
  passed in \a repl and the data of the response is passed in \a data. Do not
  call request() in response to this signal. Instead wait for finishedSuccess().

  If this QHttpClient has a device set, then this signal is not emitted.

  \sa responseChunk()
*/
/*!
  \fn void QHttpClient::response( const QHttpResponseHeader& repl, const QIODevice* device )
  \overload

  This signal is emitted when the response is available and the data was written
  to the device \a device. The response header is passed in \a repl. Do not call
  request() in response to this signal. Instead wit for finishedSuccess().

  If this QHttpClient has no device set, then this signal is not emitted.

  \sa responseChunk()
*/
/*!
  \fn void QHttpClient::responseChunk( const QHttpResponseHeader& repl, const QByteArray& data )

  This signal is emitted if the client has received a piece of the response data.
  This is useful for slow connections: you don't have to wait until all data is
  available; you can present the data that is already loaded to the user.

  The header is passed in \a repl and the data chunk in \a data.

  If you are only interested in the complete document, use one of the response()
  signals instead.

  After everything is read and reported, the finishedSuccess() signal is emitted.

  \sa finishedSuccess() response()
*/
/*!
  \fn void QHttpClient::responseHeader( const QHttpResponseHeader& repl )

  This signal is emitted if the HTTP header of the response is available. The
  header is passed in \a repl.

  It is now possible to decide wether the response data should be read in memory
  or rather in some device by calling setDevice().

  \sa setDevice() response() responseChunk()
*/
/*!
  \fn void QHttpClient::finishedError( const QString& detail, int error )

  This signal is emitted if a request failed. \a detail is a text that
  describes the reason for the error. \a error is a Error enum that
  contains the reason for the failure.

  \sa request()
*/
/*!
  \fn void QHttpClient::finishedSuccess()

  This signal is emitted when the QHttpClient is able to start a new request.
  The QHttpClient is either in the state Idle or Alive now.

  \sa response() responseChunk()
*/

/*!
    Constructs a HTTP client. The parameters \a parent and \a name are
    passed on to the QObject constructor.
*/
QHttpClient::QHttpClient( QObject* parent, const char* name )
    : QObject( parent, name ), m_state( QHttpClient::Idle ), m_idleTimer( 0 ),
      m_device( 0 ), m_postDevice( 0 )
{
    m_socket = new QSocket( this );

    connect( m_socket, SIGNAL( connected() ), this, SLOT( slotConnected() ) );
    connect( m_socket, SIGNAL( connectionClosed() ), this, SLOT( slotClosed() ) );
    connect( m_socket, SIGNAL( delayedCloseFinished() ), this, SLOT( slotClosed() ) );
    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( slotReadyRead() ) );
    connect( m_socket, SIGNAL( error( int ) ), this, SLOT( slotError( int ) ) );
    connect( m_socket, SIGNAL( bytesWritten( int ) ), this, SLOT( slotBytesWritten( int ) ) );
    connect( m_socket, SIGNAL( hostFound() ), this, SIGNAL( hostFound() ) );

    m_idleTimer = startTimer( 0 );
}

/*!
    Destructor. If there is an open connection, it is closed.
*/
QHttpClient::~QHttpClient()
{
    close();
}

/*!
    Closes the connection. This will abort a running request.

    Do not call request() in response to this signal. Instead wait for finishedSuccess().
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
	    emit closed();
	    // Prepare to emit the finishedSuccess() signal.
	    m_idleTimer = startTimer( 0 );
	}
    }
}

/*! \overload
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
    Sends a request to the server \a hostname at port \a port. Use the
    \a header as the HTTP request header. You are responsible for
    setting up a \a header that is appropriate appropriate for your
    request. \a data is a char array of size \a size; it is used as
    the content data of the HTTP request.

    Call this function after the client was created or after the
    finishedSuccess() signal is emitted. Returns TRUE if it is able to make
    the request (i.e. no request is pending); otherwise returns FALSE.
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
    \overload

    Sends a request to the server \a hostname at port \a port. Use the
    \a header as the HTTP request header. You are responsible for
    setting up a \a header that is appropriate appropriate for your
    request. The content data is read from \a device (the device must
    be opened for reading).

    Call this function after the client was created or after the
    finishedSuccess() signal is emitted. Returns TRUE if it is able to make
    the request (i.e. no request is pending); otherwise returns FALSE.
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
void QHttpClient::slotClosed()
{
    if ( m_state == Closed )
	return;

    // If the other side closed the connection then there
    // may still be data left to read.
    if ( m_state == Reading ) {
	while( m_socket->bytesAvailable() > 0 ) {
	    slotReadyRead();
	}

	// If we got no Content-Length then we know
	// now that the request has completed.
	if ( m_response.value("connection")=="close" && !m_response.hasKey( "content-length" ) ) {
	    if ( m_device )
		emit response( m_response, m_device );
	    else
		emit response( m_response, m_buffer );

	    // Save memory
	    m_buffer = QByteArray();
	} else {
	    // We got Content-Length, so did we get all bytes ?
	    if ( m_bytesRead != m_response.contentLength() ) {
		emit finishedError( tr("Wrong content length"), WrongContentLength );
	    }
	}
    } else if ( m_state == Connecting || m_state == Sending ) {
	emit finishedError( tr("Server closed connection unexpectedly"), UnexpectedClose );
    }
    emit closed();

    m_postDevice = 0;
    m_state = Closed;
    m_idleTimer = startTimer( 0 );
}

void QHttpClient::slotConnected()
{
    emit connected();

    m_state = Sending;

    QString str = m_header.toString();

    m_socket->writeBlock( str.latin1(), str.length() );
    m_socket->writeBlock( m_buffer.data(), m_buffer.size() );
    m_socket->flush();

    // Save memory
    m_buffer = QByteArray();
}

void QHttpClient::slotError( int err )
{
    m_postDevice = 0;

    if ( m_state == Connecting || m_state == Reading || m_state == Sending ) {
	switch ( err ) {
	    case QSocket::ErrConnectionRefused:
		emit finishedError( tr("Connection refused"), ConnectionRefused );
		break;
	    case QSocket::ErrHostNotFound:
		emit finishedError( tr("Host %1 not found").arg(m_socket->peerName()), HostNotFound );
		break;
	    default:
		emit finishedError( tr("HTTP request failed"), UnknownError );
		break;
	}
    }

    close();
}

void QHttpClient::slotBytesWritten( int )
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
	    m_postDevice = 0;
	}

	m_socket->writeBlock( arr.data(), max );
    }
}

void QHttpClient::slotReadyRead()
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
	    m_response = QHttpResponseHeader( QString( m_buffer ) );

	    // Check header
	    if ( !m_response.isValid() ) {
		emit finishedError( tr("Invalid HTTP response header"), InvalidResponseHeader );
		close();
		return;
	    }
	    emit responseHeader( m_response );

	    // Handle data that was already read
	    m_bytesRead = m_buffer.size() - i - 4;
	    if ( m_response.hasContentLength() )
		m_bytesRead = QMIN( m_response.contentLength(), m_bytesRead );

	    if ( m_device ) {
		// Write the data to file
		m_device->writeBlock( m_buffer.data() + i + 4, m_bytesRead );

		QByteArray tmp( m_bytesRead );
		memcpy( tmp.data(), m_buffer.data() + i + 4, m_bytesRead );
		emit responseChunk( m_response, tmp );
	    } else {
		// Copy the data to the beginning of a new buffer.
		QByteArray tmp;
		// Resize the array. Do we know the size of the data a priori ?
		if ( m_response.hasContentLength() )
		    tmp.resize( m_response.contentLength() );
		else
		    tmp.resize( m_bytesRead );
		memcpy( tmp.data(), m_buffer.data() + i + 4, m_bytesRead );
		m_buffer = tmp;

		QByteArray tmp2( m_bytesRead );
		memcpy( tmp2.data(), m_buffer.data(), m_bytesRead );
		emit responseChunk( m_response, tmp2 );
	    }
	}
    }

    if ( !m_readHeader ) {
	uint n = m_socket->bytesAvailable();
	if ( n > 0 ) {
	    if ( m_response.hasContentLength() )
		n = QMIN( m_response.contentLength() - m_bytesRead, n );

	    if ( m_device ) {
		QByteArray arr( n );
		n = m_socket->readBlock( arr.data(), n );
		m_device->writeBlock( arr.data(), n );

		arr.resize( n );
		emit responseChunk( m_response, arr );
	    } else {
		m_buffer.resize( m_buffer.size() + n );
		n = m_socket->readBlock( m_buffer.data() + m_bytesRead, n );

		QByteArray tmp( n );
		memcpy( tmp.data(), m_buffer.data()+m_bytesRead, n );
		emit responseChunk( m_response, tmp );
	    }
	    m_bytesRead += n;
	}

	// Read everything ?
	// We can only know that if the content length was given in advance.
	// Otherwise we emit the signal in closed().
	if ( m_response.hasContentLength() && m_bytesRead == m_response.contentLength() ) {
	    if ( m_device )
		emit response( m_response, m_device );
	    else
		emit response( m_response, m_buffer );

	    // Save memory
	    m_buffer = QByteArray();

	    // Handle "Connection: close"
	    if ( m_response.value("connection") == "close" ) {
		close();
	    } else {
		m_state = Alive;
		// Start a timer, so that we emit the keep alive signal
		// "after" this method returned.
		m_idleTimer = startTimer( 0 );
	    }
	}
    }
}

/*!
  \enum QHttpClient::State

    This enum is used to specify the state the client is in.

    \value Closed The connection was just closed, but is still not
    ready to accept new requests. (Wait until the client is Idle.)

    \value Connecting A request was issued and the client is looking up
    IP addresses or connecting to the remote host.

    \value Sending  The client is sending its request to the server.

    \value Reading The client has sent its request and is reading the
    server's response.

    \value Alive The connection to the host is open. It is possible to
    make new requests.

    \value Idle There is no open connection. It is possible to make new
    requests.
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
	    emit finishedSuccess();
	} else if ( m_state != Idle ) {
	    m_state = Idle;
	    emit finishedSuccess();
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

    If a device is set, then all data read by the QHttpClient (but not
    the HTTP headers) are written to this device.

    The device must be opened for writing.

    Setting the device to 0 means that subsequently read data will be
    read into memory. By default QHttpClient reads into memory.

    Setting a device makes sense when downloading very big chunks of
    data.
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
 * QHttp
 *
 ****************************************************/
/*!
    \class QHttp qhttp.h
    \brief The QHttp class provides an implementation of the HTTP protocol.

    \ingroup io
    \module network

    This class is derived from QNetworkProtocol and can be used with
    QUrlOperator. In practice this class is used through a
    QUrlOperator rather than directly, for example:
    \code
    QUrlOperator op( "http://www.trolltech.com" );
    op.get( "index.html" );
    \endcode

    Note: this code will only work if the QHttp class is registered;
    to register the class, you must call qInitNetworkProtocols()
    before using a QUrlOperator with HTTP.

    QHttp only supports the operations operationGet() and
    operationPut(), i.e. QUrlOperator::get() and QUrlOperator::put(),
    if you use it with a QUrlOperator. More control and flexibility is
    available from the QHttpClient class.

    If you really need to use QHttp directly, don't forget to set the
    QUrlOperator on which it operates using setUrl().

    \sa \link network.html Qt Network Documentation \endlink QNetworkProtocol, QUrlOperator
*/

/*!
    Constructs a QHttp object. Usually there is no need to use QHttp
    directly, since it is more convenient to use it through a
    QUrlOperator. If you want to use it directly, you must set the
    QUrlOperator on which it operates using setUrl().
*/
QHttp::QHttp()
{
    d = 0;
    bytesRead = 0;
    client = new QHttpClient( this );
    connect( client, SIGNAL(responseChunk(const QHttpResponseHeader&, const QByteArray&)),
	     this, SLOT(reply(const QHttpResponseHeader&, const QByteArray&)) );
    connect( client, SIGNAL(finishedSuccess()),
	     this, SLOT(finishedSuccess()) );
    connect( client, SIGNAL(finishedError( const QString&, int )),
	     this, SLOT(finishedError( const QString&, int )) );
    connect( client, SIGNAL(connected()),
	     SLOT(connected()) );
    connect( client, SIGNAL(closed()),
	     SLOT(closed()) );
    connect( client, SIGNAL(hostFound()),
	     SLOT(hostFound()) );
}

/*!
    Destroys the QHttp object.
*/
QHttp::~QHttp()
{
}

/*! \reimp
*/
int QHttp::supportedOperations() const
{
    return OpGet | OpPut;
}

/*! \reimp
*/
void QHttp::operationGet( QNetworkOperation *op )
{
    int cstate = client->state();
    if ( cstate != QHttpClient::Alive && cstate != QHttpClient::Idle )
	return; // ### store the request for later?

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "GET", u.encodedPathAndQuery() );
    header.setValue( "Host", u.host() );
    client->request( u.host(), u.port() != -1 ? u.port() : 80, header );
}

/*! \reimp
*/
void QHttp::operationPut( QNetworkOperation *op )
{
    int cstate = client->state();
    if ( cstate != QHttpClient::Alive && cstate != QHttpClient::Idle )
	return; // ### ditto

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "POST", u.encodedPathAndQuery() );
    // header.setContentType( "text/plain" );
    header.setValue( "Host", u.host() );
    client->request( u.host(), u.port() != -1 ? u.port() : 80, header, op->rawArg(1) );
}

void QHttp::reply( const QHttpResponseHeader &rep, const QByteArray & dataA )
{
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	if ( rep.statusCode() >= 400 && rep.statusCode() < 600 ) {
	    op->setState( StFailed );
	    op->setProtocolDetail(
		    QString("%1 %2").arg(rep.statusCode()).arg(rep.reasonPhrase())
						    );
	    switch ( rep.statusCode() ) {
		case 401:
		case 403:
		case 405:
		    op->setErrorCode( ErrPermissionDenied );
		    break;
		case 404:
		    op->setErrorCode(ErrFileNotExisting );
		    break;
		default:
		    if ( op->operation() == OpGet )
			op->setErrorCode( ErrGet );
		    else
			op->setErrorCode( ErrPut );
		    break;
	    }
	}
	// ### In cases of an error, should we still emit the data() signals?
	if ( op->operation() == OpGet && !dataA.isEmpty() ) {
	    emit data( dataA, op );
	    bytesRead += dataA.size();
	    if ( rep.hasContentLength() ) {
		emit dataTransferProgress( bytesRead, rep.contentLength(), op );
	    }
	}
    }
}

void QHttp::finishedSuccess()
{
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	if ( op->state() != StFailed ) {
	    op->setState( QNetworkProtocol::StDone );
	    op->setErrorCode( QNetworkProtocol::NoError );
	}
	emit finished( op );
    }
}

void QHttp::finishedError( const QString &detail, int err )
{
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	op->setState( QNetworkProtocol::StFailed );
	op->setProtocolDetail( detail );
	switch ( err ) {
	    case QHttpClient::ConnectionRefused:
		op->setErrorCode( ErrHostNotFound );
		break;
	    case QHttpClient::HostNotFound:
		op->setErrorCode( ErrHostNotFound );
		break;
	    default:
		if ( op->operation() == OpGet )
		    op->setErrorCode( ErrGet );
		else
		    op->setErrorCode( ErrPut );
		break;
	}
	emit finished( op );
    }
}

void QHttp::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void QHttp::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
}

void QHttp::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
}

#endif
