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


class QHttpPrivate
{
public:
    QHttpPrivate() :
	state( QHttp::Idle ),
	idleTimer( 0 ),
	device( 0 ),
	postDevice( 0 )
    { }

    QSocket* socket;
    QByteArray buffer;
    uint bytesRead;
    QHttpRequestHeader header;
    QHttp::State state;
    bool readHeader;
    QHttpResponseHeader response;

    int idleTimer;

    QIODevice* device;
    QIODevice* postDevice;
};


/****************************************************
 *
 * QHttpHeader
 *
 ****************************************************/

/*!
    \class QHttpHeader qhttp.h

    \brief The QHttpHeader class contains header information for HTTP.

    \ingroup io
    \module network

    In most cases you should use the more specialized derivatives of
    this class, QHttpResponseHeader and QHttpRequestHeader, rather than
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

    \sa QHttpRequestHeader QHttpResponseHeader
*/

/*!  \fn int QHttpHeader::majorVersion() const
  Returns the major protocol-version of the HTTP header.
*/

/*!  \fn int QHttpHeader::minorVersion() const
  Returns the minor protocol-version of the HTTP header.
*/

/*!
    Constructs an empty HTTP header.
*/
QHttpHeader::QHttpHeader()
    : valid( TRUE )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpHeader::QHttpHeader( const QHttpHeader& header )
    : valid( header.valid )
{
    values = header.values;
}

/*!
    Constructs a HTTP header for \a str.

    This constructor parses the string \a str for header fields and
    adds this information.
*/
QHttpHeader::QHttpHeader( const QString& str )
    : valid( TRUE )
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
    values = h.values;
    valid = h.valid;
    return *this;
}

/*!
    Returns TRUE if the HTTP header is valid; otherwise returns FALSE.

    A QHttpHeader is invalid if it was created by parsing a malformed string.
*/
bool QHttpHeader::isValid() const
{
    return valid;
}

/*! \internal
    Parses the HTTP header string \a str for header fields and adds
    the keys/values it finds. If the string is not parsed successfully
    the QHttpHeader becomes \link isValid() invalid\endlink.

    Returns TRUE if \a str was successfully parsed; otherwise returns FALSE.

    \sa toString()
*/
bool QHttpHeader::parse( const QString& str )
{
    QStringList lst = QStringList::split( "\r\n", str.stripWhiteSpace(), FALSE );

    if ( lst.isEmpty() )
	return TRUE;

    QStringList lines;
    QStringList::Iterator it = lst.begin();
    for( ; it != lst.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    if ( (*it)[0].isSpace() ) {
		if ( !lines.isEmpty() ) {
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
	    valid = FALSE;
	    return FALSE;
	}
    }
    return TRUE;
}

/*!
    Returns the value for the entry with the given \a key. If no entry
    has this \a key, an empty string is returned.

    \sa setValue() removeValue() hasKey() keys()
*/
QString QHttpHeader::value( const QString& key ) const
{
    return values[ key.lower() ];
}

/*!
    Returns a list of the keys in the HTTP header.

    \sa hasKey()
*/
QStringList QHttpHeader::keys() const
{
    QStringList lst;

    QMap<QString,QString>::ConstIterator it = values.begin();
    for( ; it != values.end(); ++it )
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
    return values.contains( key.lower() );
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
    values[ key.lower() ] = value;
}

/*!
    Removes the entry with the key \a key from the HTTP header.

    \sa value() setValue()
*/
void QHttpHeader::removeValue( const QString& key )
{
    values.remove( key.lower() );
}

/*! \internal
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

    values.insert( line.left( i ).stripWhiteSpace().lower(), line.mid( i + 1 ).stripWhiteSpace() );

    return TRUE;
}

/*!
    Returns a string representation of the HTTP header.
*/
QString QHttpHeader::toString() const
{
    QString ret = "";

    QMap<QString,QString>::ConstIterator it = values.begin();
    for( ; it != values.end(); ++it )
	ret += it.key() + ": " + it.data() + "\r\n";

    return ret;
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
    return values[ "content-length" ].toUInt();
}

/*!
  Sets the value of the special HTTP header field \c content-length to \a len.

  \sa contentLength() hasContentLength()
*/
void QHttpHeader::setContentLength( int len )
{
    values[ "content-length" ] = QString::number( len );
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
    QString type = values[ "content-type" ];
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
    values[ "content-type" ] = type;
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

    This class is used in the QHttp class to report the header
    information that the client received from the server.

    HTTP responses have a status code that indicates the status of the
    response. This code is a 3-digit integer result code (for details please
    refer to RFC 1945). In addition to the status code, you can also specify a
    human-readable text that describes the reason for the code ("reason
    phrase"). This class allows you to get the status code and the reason
    phrase.

    \sa QHttpRequestHeader QHttp
*/

/*!
  Constructs an empty HTTP response header.
*/
QHttpResponseHeader::QHttpResponseHeader()
{
}

/*!
  Constructs a HTTP response header with the status code \a code, the reason
  phrase \a text and the protocol-version \a majorVer and \a minorVer.
*/
QHttpResponseHeader::QHttpResponseHeader( int code, const QString& text, int majorVer, int minorVer )
    : QHttpHeader(), statCode( code ), reasonPhr( text ), majVer( majorVer ), minVer( minorVer )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpResponseHeader::QHttpResponseHeader( const QHttpResponseHeader& header )
    : QHttpHeader( header ), statCode( header.statCode ), reasonPhr( header.reasonPhr ), majVer( header.majVer ), minVer( header.minVer )
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
    the protocol-version to \a majorVer and \a minorVer.

    \sa statusCode() reasonPhrase() majorVersion() minorVersion()
*/
void QHttpResponseHeader::setStatusLine( int code, const QString& text, int majorVer, int minorVer )
{
    statCode = code;
    reasonPhr = text;
    majVer = majorVer;
    minVer = minorVer;
}

/*!
  Returns the status code of the HTTP response header.

  \sa reasonPhrase() majorVersion() minorVersion()
*/
int QHttpResponseHeader::statusCode() const
{
    return statCode;
}

/*!
  Returns the reason phrase of the HTTP response header.

  \sa statusCode() majorVersion() minorVersion()
*/
QString QHttpResponseHeader::reasonPhrase() const
{
    return reasonPhr;
}

/*!
  Returns the major protocol-version of the HTTP response header.

  \sa minorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::majorVersion() const
{
    return majVer;
}

/*!
  Returns the minor protocol-version of the HTTP response header.

  \sa majorVersion() statusCode() reasonPhrase()
*/
int QHttpResponseHeader::minorVersion() const
{
    return minVer;
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
	majVer = l[5].latin1() - '0';
	minVer = l[7].latin1() - '0';

	int pos = l.find( ' ', 9 );
	if ( pos != -1 ) {
	    reasonPhr = l.mid( pos + 1 );
	    statCode = l.mid( 9, pos - 9 ).toInt();
	} else {
	    statCode = l.mid( 9 ).toInt();
	    reasonPhr = QString::null;
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
    return ret.arg( majVer ).arg ( minVer ).arg( statCode ).arg( reasonPhr ).arg( QHttpHeader::toString() );
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

    \ingroup io
    \module network

    This class is used in the QHttp class to report the header
    information if the client requests something from the server.

    HTTP requests have a method which describes the request's action.
    The most common requests are "GET" and "POST". In addition to the
    request method the header also includes a request-URI to specify
    the location for the method to use.

    The method, request-URI and protocol-version can be set using a
    constructor or later using setRequest(). The values can be
    obtained using method(), path(), majorVersion() and minorVersion().

    This class is a QHttpHeader subclass so that class's functions,
    e.g. \link QHttpHeader::setValue() setValue()\endlink, \link
    QHttpHeader::value() value()\endlink, etc. are also available.


    \sa QHttpResponseHeader QHttp
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
    request-URI \a path and the protocol-version \a majorVer and \a minorVer.
*/
QHttpRequestHeader::QHttpRequestHeader( const QString& method, const QString& path, int majorVer, int minorVer )
    : QHttpHeader(), m( method ), p( path ), majVer( majorVer ), minVer( minorVer )
{
}

/*!
    Constructs a copy of \a header.
*/
QHttpRequestHeader::QHttpRequestHeader( const QHttpRequestHeader& header )
    : QHttpHeader( header ), m( header.m ), p( header.p ), majVer( header.majVer ), minVer( header.minVer )
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
    request-URI to \a path and the protocol-version to \a majorVer and \a
    minorVer.

    \sa method() path() majorVersion() minorVersion()
*/
void QHttpRequestHeader::setRequest( const QString& method, const QString& path, int majorVer, int minorVer )
{
    m = method;
    p = path;
    majVer = majorVer;
    minVer = minorVer;
}

/*!
    Returns the method of the HTTP request header.

    \sa path() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::method() const
{
    return m;
}

/*!
    Returns the request-URI of the HTTP request header.

    \sa method() majorVersion() minorVersion() setRequest()
*/
QString QHttpRequestHeader::path() const
{
    return p;
}

/*!
  Returns the major protocol-version of the HTTP request header.

  \sa minorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::majorVersion() const
{
    return majVer;
}

/*!
  Returns the minor protocol-version of the HTTP request header.

  \sa majorVersion() method() path() setRequest()
*/
int QHttpRequestHeader::minorVersion() const
{
    return minVer;
}

/*! \reimp
*/
bool QHttpRequestHeader::parseLine( const QString& line, int number )
{
    if ( number != 0 )
	return QHttpHeader::parseLine( line, number );

    QStringList lst = QStringList::split( " ", line.simplifyWhiteSpace() );
    if ( lst.count() > 0 ) {
	m = lst[0];
	if ( lst.count() > 1 ) {
	    p = lst[1];
	    if ( lst.count() > 2 ) {
		QString v = lst[2];
		if ( v.length() >= 8 && v.left( 5 ) == "HTTP/" &&
			v[5].isDigit() && v[6] == '.' && v[7].isDigit() ) {
		    majVer = v[5].latin1() - '0';
		    minVer = v[7].latin1() - '0';
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
    return first.arg( m ).arg( p ) +
	last.arg( majVer ).arg( minVer ).arg( QHttpHeader::toString());
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


    ### merge the following documentation nicely with the above

    This class provides the functionality required to send HTTP
    requests to an HTTP server and to receive replies from the server.
    It provides full control over the request header and full access
    to the response header.

    Note that the QHttp class provides a much easier API for fetching
    single URIs (it provides the QUrlOperator interface for HTTP) and
    is more suitable for simple requirements.

    To make an HTTP request you must set up suitable HTTP headers. The
    following example demonstrates, how to request the main HTML page
    from the Trolltech home page (i.e. the URL
    http://www.trolltech.com/index.html):
    \code
    QHttp client;
    QHttpRequestHeader header( "GET", "/index.html" );
    client.request( "www.trolltech.com", 80, header );
    \endcode
    \omit WE SHOULD SHOW A CONNECTION\endomit

    This only makes the request. If a part of the response arrived, the
    signal responseChunk() is emitted. After the last chunk is reported,
    the finishedSuccess() signal is emitted. This allows you to process the
    document as chunks are received, without having to wait for the
    complete transmission to be finished.

    If you need to receive the complete document before continuing,
    you might prefer to connect to the response() signal which is emitted
    when the entire document is received.

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
    init();
}

void QHttp::init()
{
    bytesRead = 0;
    d = new QHttpPrivate;

    // ################## signal/slot connections used in the QUrlOperator mode
    connect( this, SIGNAL(responseChunk(const QHttpResponseHeader&, const QByteArray&)),
	     SLOT(clientReply(const QHttpResponseHeader&, const QByteArray&)) );
    connect( this, SIGNAL(finishedSuccess()),
	     SLOT(clientFinishedSuccess()) );
    connect( this, SIGNAL(finishedError( const QString&, int )),
	     SLOT(clientFinishedError( const QString&, int )) );
    connect( this, SIGNAL(connected()),
	     SLOT(clientConnected()) );
    connect( this, SIGNAL(closed()),
	     SLOT(clientClosed()) );
    connect( this, SIGNAL(hostFound()),
	     SLOT(clientHostFound()) );

    // new API
    d->socket = new QSocket( this );

    connect( d->socket, SIGNAL( connected() ),
	    this, SLOT( slotConnected() ) );
    connect( d->socket, SIGNAL( connectionClosed() ),
	    this, SLOT( slotClosed() ) );
    connect( d->socket, SIGNAL( delayedCloseFinished() ),
	    this, SLOT( slotClosed() ) );
    connect( d->socket, SIGNAL( readyRead() ),
	    this, SLOT( slotReadyRead() ) );
    connect( d->socket, SIGNAL( error( int ) ),
	    this, SLOT( slotError( int ) ) );
    connect( d->socket, SIGNAL( bytesWritten( int ) ),
	    this, SLOT( slotBytesWritten( int ) ) );
    connect( d->socket, SIGNAL( hostFound() ),
	    this, SIGNAL( hostFound() ) );

    d->idleTimer = startTimer( 0 );
}

/*!
    Destroys the QHttp object. If there is an open connection, it is closed.
*/
QHttp::~QHttp()
{
    close();
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
    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "GET", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    request( u.host(), u.port() != -1 ? u.port() : 80, header );
}

/*! \reimp
*/
void QHttp::operationPut( QNetworkOperation *op )
{
    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "POST", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    request( u.host(), u.port() != -1 ? u.port() : 80, header, op->rawArg(1) );
}

void QHttp::clientReply( const QHttpResponseHeader &rep, const QByteArray & dataA )
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

void QHttp::clientFinishedSuccess()
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

void QHttp::clientFinishedError( const QString &detail, int err )
{
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	op->setState( QNetworkProtocol::StFailed );
	op->setProtocolDetail( detail );
	switch ( err ) {
	    case ConnectionRefused:
		op->setErrorCode( ErrHostNotFound );
		break;
	    case HostNotFound:
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

void QHttp::clientHostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void QHttp::clientConnected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
}

void QHttp::clientClosed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
}

#endif

/****************************************************
 *
 * QHttp -- new API
 *
 ****************************************************/

/*!
  \fn void QHttp::response( const QHttpResponseHeader& repl, const QByteArray& data )

  This signal is emitted when the response is available. The response header is
  passed in \a repl and the data of the response is passed in \a data. Do not
  call request() in response to this signal. Instead wait for finishedSuccess().

  If this QHttp has a device set, then this signal is not emitted.

  \sa responseChunk()
*/
/*!
  \fn void QHttp::response( const QHttpResponseHeader& repl, const QIODevice* device )
  \overload

  This signal is emitted when the response is available and the data was written
  to the device \a device. The response header is passed in \a repl. Do not call
  request() in response to this signal. Instead wit for finishedSuccess().

  If this QHttp has no device set, then this signal is not emitted.

  \sa responseChunk()
*/
/*!
  \fn void QHttp::responseChunk( const QHttpResponseHeader& repl, const QByteArray& data )

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
  \fn void QHttp::responseHeader( const QHttpResponseHeader& repl )

  This signal is emitted if the HTTP header of the response is available. The
  header is passed in \a repl.

  It is now possible to decide wether the response data should be read in memory
  or rather in some device by calling setDevice().

  \sa setDevice() response() responseChunk()
*/
/*!
  \fn void QHttp::finishedError( const QString& detail, int error )

  This signal is emitted if a request failed. \a detail is a text that
  describes the reason for the error. \a error is a Error enum that
  contains the reason for the failure.

  \sa request()
*/
/*!
  \fn void QHttp::finishedSuccess()

  This signal is emitted when the QHttp is able to start a new request.
  The QHttp is either in the state Idle or Alive now.

  \sa response() responseChunk()
*/

/*!
    Constructs a HTTP client. The parameters \a parent and \a name are
    passed on to the QObject constructor.
*/
QHttp::QHttp( QObject* parent, const char* name )
{
    if ( parent )
	parent->insertChild( this );
    setName( name );
    init();
}


/*!
    Closes the connection. This will abort a running request.

    Do not call request() in response to this signal. Instead wait for finishedSuccess().
*/
void QHttp::close()
{
    // If no connection is open -> ignore
    if ( d->state == Closed || d->state == Idle )
	return;

    d->postDevice = 0;
    d->state = Closed;

    // Already closed ?
    if ( !d->socket->isOpen() ) {
	d->idleTimer = startTimer( 0 );
    } else {
	// Close now.
	d->socket->close();

	// Did close succeed immediately ?
	if ( d->socket->state() == QSocket::Idle ) {
	    emit closed();
	    // Prepare to emit the finishedSuccess() signal.
	    d->idleTimer = startTimer( 0 );
	}
    }
}

/*! \overload
*/
bool QHttp::request( const QString& hostname, int port, const QHttpRequestHeader& header )
{
    return request( hostname, port, header, QByteArray(), 0 );
}

/*! \overload
*/
bool QHttp::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QByteArray& data )
{
    return request( hostname, port, header, data, data.size() );
}

/*! \overload
*/
bool QHttp::request( const QString& hostname, int port, const QHttpRequestHeader& header, const QCString& data )
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
bool QHttp::request( const QString& hostname, int port, const QHttpRequestHeader& header, const char* data, uint size )
{
    // Is it allowed to make a new request now ?
    if ( d->state != Idle && d->state != Alive ) {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    d->state = Connecting;

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( d->socket->peerName() != hostname || d->socket->state() != QSocket::Connection ) {
	d->socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the data
    d->buffer.duplicate( data, size );
    d->header = header;

    if ( d->buffer.size() > 0 )
	d->header.setContentLength( d->buffer.size() );

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
bool QHttp::request( const QString& hostname, int port, const QHttpRequestHeader& header, QIODevice* device )
{
    if ( d->state != Idle ) {
	qWarning("The client is currently busy with a pending request. You can not invoke a request now.");
	return FALSE;
    }

    killIdleTimer();
    d->state = Connecting;

    d->postDevice = device;
    if ( !d->postDevice || !d->postDevice->isOpen() || !d->postDevice->isReadable() ) {
	qWarning("The device passes to QHttp::request must be opened for reading");
	return FALSE;
    }

    d->buffer = QByteArray();

    // Do we need to setup a new connection or can we reuse an
    // existing one ?
    if ( d->socket->peerName() != hostname || d->socket->state() != QSocket::Connection ) {
	d->socket->connectToHost( hostname, port );
    }

    // Get a deep copy of the header
    d->header = header;
    d->header.setContentLength( d->postDevice->size() );

    return TRUE;
}

/*!
    Some cleanup if the connection got closed.
*/
void QHttp::slotClosed()
{
    if ( d->state == Closed )
	return;

    // If the other side closed the connection then there
    // may still be data left to read.
    if ( d->state == Reading ) {
	while( d->socket->bytesAvailable() > 0 ) {
	    slotReadyRead();
	}

	// If we got no Content-Length then we know
	// now that the request has completed.
	if ( d->response.value("connection")=="close" && !d->response.hasKey( "content-length" ) ) {
	    if ( d->device )
		emit response( d->response, d->device );
	    else
		emit response( d->response, d->buffer );

	    // Save memory
	    d->buffer = QByteArray();
	} else {
	    // We got Content-Length, so did we get all bytes ?
	    if ( d->bytesRead != d->response.contentLength() ) {
		emit finishedError( tr("Wrong content length"), WrongContentLength );
	    }
	}
    } else if ( d->state == Connecting || d->state == Sending ) {
	emit finishedError( tr("Server closed connection unexpectedly"), UnexpectedClose );
    }
    emit closed();

    d->postDevice = 0;
    d->state = Closed;
    d->idleTimer = startTimer( 0 );
}

void QHttp::slotConnected()
{
    emit connected();

    d->state = Sending;

    QString str = d->header.toString();

    d->socket->writeBlock( str.latin1(), str.length() );
    d->socket->writeBlock( d->buffer.data(), d->buffer.size() );
    d->socket->flush();

    // Save memory
    d->buffer = QByteArray();
}

void QHttp::slotError( int err )
{
    d->postDevice = 0;

    if ( d->state == Connecting || d->state == Reading || d->state == Sending ) {
	switch ( err ) {
	    case QSocket::ErrConnectionRefused:
		emit finishedError( tr("Connection refused"), ConnectionRefused );
		break;
	    case QSocket::ErrHostNotFound:
		emit finishedError( tr("Host %1 not found").arg(d->socket->peerName()), HostNotFound );
		break;
	    default:
		emit finishedError( tr("HTTP request failed"), UnknownError );
		break;
	}
    }

    close();
}

void QHttp::slotBytesWritten( int )
{
    if ( !d->postDevice )
	return;

    if ( d->socket->bytesToWrite() == 0 ) {
	int max = QMIN( 4096, d->postDevice->size() - d->postDevice->at() );
	QByteArray arr( max );

	int n = d->postDevice->readBlock( arr.data(), max );
	if ( n != max ) {
	    qWarning("Could not read enough bytes from the device");
	    close();
	    return;
	}
	if ( d->postDevice->atEnd() ) {
	    d->postDevice = 0;
	}

	d->socket->writeBlock( arr.data(), max );
    }
}

void QHttp::slotReadyRead()
{
    if ( d->state != Reading ) {
	d->state = Reading;
	d->buffer = QByteArray();
	d->readHeader = TRUE;
    }

    if ( d->readHeader ) {
	int n = d->socket->bytesAvailable();
	if ( n == 0 )
	    return;

	int s = d->buffer.size();
	d->buffer.resize( s + n );
	n = d->socket->readBlock( d->buffer.data() + s, n );

	// Search for \r\n\r\n
	const char* data = d->buffer.data();
	int i;
	bool end = FALSE;
	for( i = QMAX( 0, s - 3 ); !end && i+3 < s + n; ++i ) {
	    if ( data[i] == '\r' && data[i+1] == '\n' && data[i+2] == '\r' && data[i+3] == '\n' )
		end = TRUE;
	}

	if ( end ) {
	    --i;
	    d->readHeader = FALSE;
	    d->buffer[i] = 0;
	    d->response = QHttpResponseHeader( QString( d->buffer ) );

	    // Check header
	    if ( !d->response.isValid() ) {
		emit finishedError( tr("Invalid HTTP response header"), InvalidResponseHeader );
		close();
		return;
	    }
	    emit responseHeader( d->response );

	    // Handle data that was already read
	    d->bytesRead = d->buffer.size() - i - 4;
	    if ( d->response.hasContentLength() )
		d->bytesRead = QMIN( d->response.contentLength(), d->bytesRead );

	    if ( d->device ) {
		// Write the data to file
		d->device->writeBlock( d->buffer.data() + i + 4, d->bytesRead );

		QByteArray tmp( d->bytesRead );
		memcpy( tmp.data(), d->buffer.data() + i + 4, d->bytesRead );
		emit responseChunk( d->response, tmp );
	    } else {
		// Copy the data to the beginning of a new buffer.
		QByteArray tmp;
		// Resize the array. Do we know the size of the data a priori ?
		if ( d->response.hasContentLength() )
		    tmp.resize( d->response.contentLength() );
		else
		    tmp.resize( d->bytesRead );
		memcpy( tmp.data(), d->buffer.data() + i + 4, d->bytesRead );
		d->buffer = tmp;

		QByteArray tmp2( d->bytesRead );
		memcpy( tmp2.data(), d->buffer.data(), d->bytesRead );
		emit responseChunk( d->response, tmp2 );
	    }
	}
    }

    if ( !d->readHeader ) {
	uint n = d->socket->bytesAvailable();
	if ( n > 0 ) {
	    if ( d->response.hasContentLength() )
		n = QMIN( d->response.contentLength() - d->bytesRead, n );

	    if ( d->device ) {
		QByteArray arr( n );
		n = d->socket->readBlock( arr.data(), n );
		d->device->writeBlock( arr.data(), n );

		arr.resize( n );
		emit responseChunk( d->response, arr );
	    } else {
		d->buffer.resize( d->buffer.size() + n );
		n = d->socket->readBlock( d->buffer.data() + d->bytesRead, n );

		QByteArray tmp( n );
		memcpy( tmp.data(), d->buffer.data()+d->bytesRead, n );
		emit responseChunk( d->response, tmp );
	    }
	    d->bytesRead += n;
	}

	// Read everything ?
	// We can only know that if the content length was given in advance.
	// Otherwise we emit the signal in closed().
	if ( d->response.hasContentLength() && d->bytesRead == d->response.contentLength() ) {
	    if ( d->device )
		emit response( d->response, d->device );
	    else
		emit response( d->response, d->buffer );

	    // Save memory
	    d->buffer = QByteArray();

	    // Handle "Connection: close"
	    if ( d->response.value("connection") == "close" ) {
		close();
	    } else {
		d->state = Alive;
		// Start a timer, so that we emit the keep alive signal
		// "after" this method returned.
		d->idleTimer = startTimer( 0 );
	    }
	}
    }
}

/*!
  \enum QHttp::State

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
QHttp::State QHttp::state() const
{
    return d->state;
}

/*! \reimp
*/
void QHttp::timerEvent( QTimerEvent *e )
{
    if ( e->timerId() == d->idleTimer ) {
	killTimer( d->idleTimer );
	d->idleTimer = 0;

	if ( d->state == Alive ) {
	    emit finishedSuccess();
	} else if ( d->state != Idle ) {
	    d->state = Idle;
	    emit finishedSuccess();
	}
    } else {
	QObject::timerEvent( e );
    }
}

void QHttp::killIdleTimer()
{
    killTimer( d->idleTimer );
    d->idleTimer = 0;
}

/*!
    Sets the device of the HTTP client to \a dev.

    If a device is set, then all data read by the QHttp (but not
    the HTTP headers) are written to this device.

    The device must be opened for writing.

    Setting the device to 0 means that subsequently read data will be
    read into memory. By default QHttp reads into memory.

    Setting a device makes sense when downloading very big chunks of
    data.
*/
void QHttp::setDevice( QIODevice* dev )
{
    if ( dev == d->device )
	return;

    if ( !dev->isOpen() || !dev->isWritable() ) {
	qWarning("The socket must be opened for writing");
	return;
    }

    if ( d->device == 0 ) {
	if ( d->state == Reading && !d->readHeader ) {
	    dev->writeBlock( d->buffer.data(), d->bytesRead );
	}
    }

    d->device = dev;
}

/*!
    Returns the device of the HTTP client.
*/
QIODevice* QHttp::device() const
{
    return d->device;
}
