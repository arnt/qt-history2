/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurl.cpp#62 $
**
** Implementation of QUrl class
**
** Created : 950429
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qurl.h"
#include "qnetworkprotocol.h"

#include <stdlib.h>

struct QUrlPrivate
{
    QString protocol;
    QString user;
    QString pass;
    QString host;
    QString path, cleanPath;
    QString refEncoded;
    QString queryEncoded;
    bool isValid;
    int port;
    bool cleanPathDirty;
};

static void slashify( QString& s )
{
    for ( int i = 0; i < (int)s.length(); i++ ) {
	if ( s[ i ] == '\\' )
	    s[ i ] = '/';
    }
}

// NOT REVISED
/*!
  \class QUrl qurl.h

  \brief This class provides mainly an URL parser and
  simplifies working with URLs.

  The QUrl class is provided for a easy working with URLs.
  It does all parsing, decoding, encoding and so on.

  Mention that URL has some restrictions regarding the path
  encoding. URL works intern with the decoded path and
  and encoded query. For example in

  http://localhost/cgi-bin/test%20me.pl?cmd=Hello%20you

  would result in a decoded path "/cgi-bin/test me.pl"
  and in the encoded query "cmd=Hello%20you".
  Since path is internally always encoded you may NOT use
  "%00" in the path while this is ok for the query.

  If you want to use an URL to work on a hirachical filesystem
  (locally or remote) the class QUrlOperator, which is derived
  fro QUrl, may be of interest.

  \sa QUrlOperator::QUrlOperator()
*/


/*!
  Constructs an empty URL which is invalid.
*/

QUrl::QUrl()
{
    d = new QUrlPrivate;
    d->isValid = FALSE;
    d->port = -1;
    d->cleanPathDirty = TRUE;
}

/*!
  Constructs and URL using \a url and parses this string.

  \a url is considered to be encoded. You can pass strings like
  "/home/qt", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Troll%20Tech" will be decoded to
  "/home/Troll Tech". This means: If you have a usual UNIX like
  path, you have to use \link encode first before you pass it to URL.
*/

QUrl::QUrl( const QString& url )
{
    d = new QUrlPrivate;
    d->protocol = "file";
    d->port = -1;
    parse( url );
}

/*!
  Copy constructor. Copies the data or \url.
*/

QUrl::QUrl( const QUrl& url )
{
    d = new QUrlPrivate;
    *d = *url.d;
}

/*!
  Returns TRUE, if \a url is relative, else it returns FALSE.
*/

bool QUrl::isRelativeUrl( const QString &url )
{
    int colon = url.find( ":" );
    int slash = url.find( "/" );

    return ( slash != 0 && ( colon == -1 || ( slash != -1 && colon > slash ) ) );
}

/*!
  Constructs and URL taking \a url as base and \a relUrl_ as
  relative URL to \a url. If \a relUrl_ is not relative,
  \a relUrl_ is taken as new URL.
*/

QUrl::QUrl( const QUrl& url, const QString& relUrl_ )
{
    d = new QUrlPrivate;
    QString relUrl = relUrl_.stripWhiteSpace();
    slashify( relUrl );
    
    if ( !isRelativeUrl( relUrl ) ) {
	if ( relUrl[ 0 ] == QChar( '/' ) ) {
	    *this = url;
	    setEncodedPathAndQuery( relUrl );
	} else {
	    *this = relUrl;
	}
    } else {
	if ( relUrl[ 0 ] == '#' ) {
	    *this = url;
	    relUrl.remove( 0, 1 );
	    decode( relUrl );
	    setRef( relUrl );
	} else {
	    decode( relUrl );
	    *this = url;
	    QString p = url.path();
	    if ( p.isEmpty() )
		p = "/";
	    if ( p.right( 1 ) != "/" )
		p += "/";
	    p += relUrl;
	    d->cleanPathDirty = TRUE;
	    d->path = p;
	}
    }
}

/*!
  Destructor.
*/

QUrl::~QUrl()
{
    delete d;
}

/*!
  Returns the protocol of the URL.
*/

QString QUrl::protocol() const
{
    return d->protocol;
}

/*!
  Sets the protocol of the URL. This could be e.g.
  "file", "ftp" or something similar.
*/

void QUrl::setProtocol( const QString& protocol )
{
    d->protocol = protocol;
}

/*!
  Returns the username of the URL.
*/

QString QUrl::user() const
{
    return  d->user;
}

/*!
  Sets the username of the URL.
*/

void QUrl::setUser( const QString& user )
{
    d->user = user;
}

/*!
  Returns TRUE, if the URL contains an username,
  else FALSE;
*/

bool QUrl::hasUser() const
{
    return !d->user.isEmpty();
}

/*!
  Returns the password of the URL.
*/

QString QUrl::pass() const
{
    return d->pass;
}

/*!
  Sets the password of the URL.
*/

void QUrl::setPass( const QString& pass )
{
    d->pass = pass;
}

/*!
  Returns TRUE, if the URL contains a password,
  else FALSE;
*/

bool QUrl::hasPass() const
{
    return !d->pass.isEmpty();
}

/*!
  Returns the hostname of the URL.
*/

QString QUrl::host() const
{
    return d->host;
}

/*!
  Sets the hostname of the URL.
*/

void QUrl::setHost( const QString& host )
{
    d->host = host;
}

/*!
  Returns TRUE, if the URL contains a hostname,
  else FALSE;
*/

bool QUrl::hasHost() const
{
    return !d->host.isEmpty();
}

/*!
  Returns the port of the URL.
*/

int QUrl::port() const
{
    return d->port;
}

/*!
  Sets the port of the URL.
*/

void QUrl::setPort( int port )
{
    d->port = port;
}

/*!
  Sets the path or the URL.
*/

void QUrl::setPath( const QString& path )
{
    d->path = path;
    slashify( d->path );
    d->cleanPathDirty = TRUE;
}

/*!
  Returns TRUE, if the URL contains a path,
  else FALSE.
*/

bool QUrl::hasPath() const
{
    return !d->path.isEmpty();
}

/*!
  Sets the query of the URL. Must be encoded.
*/

void QUrl::setQuery( const QString& txt )
{
    d->queryEncoded = txt;
}

/*!
  Returns the query (encoded) of the URL.
*/

QString QUrl::query() const
{ 	
    return d->queryEncoded;
}

/*!
  Returns the reference (encoded) of the URL.
*/

QString QUrl::ref() const
{
    return d->refEncoded;
}

/*!
  Sets the reference of the URL. Must be encoded.
*/

void QUrl::setRef( const QString& txt )
{
    d->refEncoded = txt;
}

/*!
  Returns TRUE, if the URL has a reference, else
  it returnd FALSE;
*/

bool QUrl::hasRef() const
{
    return !d->refEncoded.isEmpty();
}

/*!
  Returns TRUE if the URL is valid, else FALSE.
  An URL is e.g. invalid if there was a parse error.
*/

bool QUrl::isValid() const
{
    return d->isValid;
}

/*!
  Resets all values if the URL to its default values
  and invalidates it.
*/

void QUrl::reset()
{
    d->protocol = "file";
    d->user = "";
    d->pass = "";
    d->host = "";
    d->path = "";
    d->queryEncoded = "";
    d->refEncoded = "";
    d->isValid = TRUE;
    d->port = -1;
    d->cleanPathDirty = TRUE;
}

/*!
  Parses the \a url.
*/

bool QUrl::parse( const QString& url )
{
    QString url_( url );
    slashify( url_ );
    
    if ( url_.isEmpty() ) {
	d->isValid = FALSE;
	return FALSE;
    }

    d->cleanPathDirty = TRUE;
    d->isValid = TRUE;
    QString oldProtocol = d->protocol;
    d->protocol = QString::null;

    const int Init 	= 0;
    const int Protocol 	= 1;
    const int Separator1= 2; // :
    const int Separator2= 3; // :/
    const int Separator3= 4; // :// or more slashes
    const int User 	= 5;
    const int Pass 	= 6;
    const int Host 	= 7;
    const int Path 	= 8;
    const int Ref 	= 9;
    const int Query 	= 10;
    const int Port 	= 11;
    const int Done 	= 12;

    const int InputAlpha= 1;
    const int InputDigit= 2;
    const int InputSlash= 3;
    const int InputColon= 4;
    const int InputAt 	= 5;
    const int InputHash = 6;
    const int InputQuery= 7;

    static uchar table[ 12 ][ 8 ] = {
     /* None       InputAlpha  InputDigit  InputSlash  InputColon  InputAt     InputHash   InputQuery */ 	
	{ 0,       Protocol,   0,          Path,       0,          0,          0,          0,         }, // Init
	{ 0,       Protocol,   0,          0,          Separator1, 0,          0,          0,         }, // Protocol
	{ 0,       0,          0,          Separator2, 0,          0,          0,          0,         }, // Separator1
	{ 0,       Path,       0,          Separator3, 0,          0,          0,          0,         }, // Separator2
	{ 0,       User,       0,          Separator3, Pass,       Host,       0,          0,         }, // Separator3
	{ 0,       User,       User,       User,       Pass,       Host,       User,       User,      }, // User
	{ 0,       Pass,       Pass,       Pass,       Pass,       Host,       Pass,       Pass,      }, // Pass
	{ 0,       Host,       Host,       Path,       Port,       Host,       Ref,        Query,     }, // Host
	{ 0,       Path,       Path,       Path,       Path,       Path,       Ref,        Query,     }, // Path
	{ 0,       Ref,        Ref,        Ref,        Ref,        Ref,        Ref,        Query,     }, // Ref
	{ 0,       Query,      Query,      Query,      Query,      Query,      Query,      Query,     }, // Query
	{ 0,       0,          Port,       Path,       0,          0,          0,          0,         }  // Port
    };

    bool relPath = FALSE;

    relPath = FALSE;
    bool forceRel = FALSE;

    // if :/ is at pos 1, we have only one letter
    // before that separator => that's a drive letter!
    if ( url_.find( ":/" ) == 1 )
	relPath = forceRel = TRUE;

    int cs = url_.find( ":/" );
    table[ 4 ][ 1 ] = User;
    if ( cs == -1 || forceRel ) { // we have a relative file (no path, host, protocol, etc.)
	table[ 0 ][ 1 ] = Path;
	relPath = TRUE;
    } else { // some checking
	table[ 0 ][ 1 ] = Protocol;
	
	// find the part between the protocol and the path as the meaning
	// of that part is dependend on some chars
	++cs;
	while ( url_[ cs ] == '/' )
	    ++cs;
	int slash = url_.find( "/", cs );
	if ( slash == -1 )
	    slash = url_.length() - 1;
	QString tmp = url_.mid( cs, slash - cs + 1 );
	
	if ( !tmp.isEmpty() ) { // if this part exists
	
	    // look for the @ in this part
	    int at = tmp.find( "@" );
	    if ( at != -1 )
		at += cs;
	    // we have no @, which means
	    // host[:port], so directly after the protocol the host starts,
	    // or if the protocol is file, it´s the path
	    if ( at == -1 ) {
		if ( url_.left( 4 ) == "file" )
		    table[ 4 ][ 1 ] = Path;
		else
		    table[ 4 ][ 1 ] = Host;
	    }
	}
    }
	
    int state = Init; // parse state
    int input; // input token

    QChar c = url_[ 0 ];
    int i = 0;
    QString port;

    while ( TRUE ) {
	
	switch ( c ) {
	case '?':
	    input = InputQuery;
	    break;
	case '#':
	    input = InputHash;
	    break;
	case '@':
	    input = InputAt;
	
	    break;
	case ':':
	    input = InputColon;
	    break;
	case '/':
	    input = InputSlash;
	    break;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9': case '0':
	    input = InputDigit;
	    break;
	default:
	    input = InputAlpha;
	}

    	state = table[ state ][ input ];
	
	switch ( state ) {
	case Protocol:
	    d->protocol += c;
	    break;
	case User:
	    d->user += c;
	    break;
	case Pass:
	    d->pass += c;
	    break;
	case Host:
	    d->host += c;
	    break;
	case Path:
	    d->path += c;
	    break;
	case Ref:
	    d->refEncoded += c;
	    break;
	case Query:
	    d->queryEncoded += c;
	    break;
	case Port:
	    port += c;
	    break;
	default:
	    break;
	}
	
	++i;
	if ( i > (int)url_.length() - 1 || state == Done || state == 0 )
	    break;
	c = url_[ i ];
	
    }

    if ( !port.isEmpty() ) {
	port.remove( 0, 1 );
	d->port = atoi( port.latin1() );
    }

    // error
    if ( i < (int)url_.length() - 1 ) {
	d->isValid = FALSE;
	return FALSE;
    }
	

    if ( d->protocol.isEmpty() )
	d->protocol = oldProtocol;

    if ( d->path.isEmpty() )
	d->path = "/";
    
    // hack for windows
    if ( d->path.length() == 2 && d->path[ 1 ] == ':' )
	d->path += "/";

    // #### do some corrections, should be done nicer too
    if ( !d->pass.isEmpty() && d->pass[ 0 ] == ':' )
	d->pass.remove( 0, 1 );
    if ( !d->path.isEmpty() ) {
	if ( d->path[ 0 ] == '@' || d->path[ 0 ] == ':' )
	    d->path.remove( 0, 1 );
	if ( d->path[ 0 ] != '/' && !relPath && d->path[ 1 ] != ':' )
	    d->path.prepend( "/" );
    }
    if ( !d->refEncoded.isEmpty() && d->refEncoded[ 0 ] == '#' )
	d->refEncoded.remove( 0, 1 );
    if ( !d->queryEncoded.isEmpty() && d->queryEncoded[ 0 ] == '?' )
	d->queryEncoded.remove( 0, 1 );
    if ( !d->host.isEmpty() && d->host[ 0 ] == '@' )
	d->host.remove( 0, 1 );

    decode( d->path );
    d->cleanPathDirty = TRUE;

#if 0
    qDebug( "URL: %s", url.latin1() );
    qDebug( "protocol: %s", d->protocol.latin1() );
    qDebug( "user: %s", d->user.latin1() );
    qDebug( "pass: %s", d->pass.latin1() );
    qDebug( "host: %s", d->host.latin1() );
    qDebug( "path: %s", path().latin1() );
    qDebug( "ref: %s", d->refEncoded.latin1() );
    qDebug( "query: %s", d->queryEncoded.latin1() );
    qDebug( "port: %d\n\n----------------------------\n\n", d->port );
#endif

    return TRUE;
}

/*!
  Assign operator. Parses \a url and assigns the resulting
  data to this class.

  \a url is considered to be encoded. You can pass strings like
  "/home/qt", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Troll%20Tech" will be decoded to
  "/home/Troll Tech". This means: If you have a usual UNIX like
  path, you have to use \link encode first before you pass it to URL.
*/

QUrl& QUrl::operator=( const QString& url )
{
    reset();
    parse( url );

    return *this;
}

/*!
  Assign operator. Assigns the data of \a url to this class.
*/

QUrl& QUrl::operator=( const QUrl& url )
{
    *d = *url.d;
    return *this;
}

/*!
  Compares this URL with \a url.
*/

bool QUrl::operator==( const QUrl& url ) const
{
    if ( !isValid() || !url.isValid() )
	return FALSE;

    if ( d->protocol == url.d->protocol &&
	 d->user == url.d->user &&
	 d->pass == url.d->pass &&
	 d->host == url.d->host &&
	 d->path == url.d->path &&
	 d->queryEncoded == url.d->queryEncoded &&
	 d->refEncoded == url.d->refEncoded &&
	 d->isValid == url.d->isValid &&
	 d->port == url.d->port )
	return TRUE;

    return FALSE;
}

/*!
  Compares this URL with \a url. \a url is parsed
  first.
*/

bool QUrl::operator==( const QString& url ) const
{
    QUrl u( url );
    return ( *this == u );
}

/*!
  Sets the filename of the URL to \a name.
*/

void QUrl::setFileName( const QString& name )
{
    QString fn( name );
    slashify( fn );
    
    while ( fn[ 0 ] == '/' )
	fn.remove( 0, 1 );

    QString p = d->path.isEmpty() ?
		QString( "/" ) : d->path;
    if ( !d->path.isEmpty() ) {
	int slash = p.findRev( QChar( '/' ) );
	if ( slash == -1 ) {
	    p = "/";
    } else if ( p.right( 1 ) != "/" )
	p.truncate( slash + 1 );
    }

    p += fn;
    setEncodedPathAndQuery( p );
}

/*!
  Returns the encoded path plus the query (encoded too).
*/

QString QUrl::encodedPathAndQuery()
{
    QString p = path();
    if ( p.isEmpty() )
	p = "/";

    encode( p );

    if ( !d->queryEncoded.isEmpty() ) {
	p += "?";
	p += d->queryEncoded;
    }

    return p;
}

/*!
  Sets path and query. Both have to be encoded.
*/

void QUrl::setEncodedPathAndQuery( const QString& path )
{
    d->cleanPathDirty = TRUE;
    int pos = path.find( '?' );
    if ( pos == -1 ) {
	d->path = path;
	d->queryEncoded = "";
    } else {
	d->path = path.left( pos );
	d->queryEncoded = path.mid( pos + 1 );
    }

    decode( d->path );
    d->cleanPathDirty = TRUE;
}

/*!
  Returns the path of the URL. If \a correct is TRUE,
  the path is cleaned (dealing with too many or few
  slashes, etc). Else exactly the path which was parsed
  or set is returned.
*/

QString QUrl::path( bool correct ) const
{
    if ( !correct )
	return d->path;

    if ( d->cleanPathDirty ) {
	if ( isLocalFile() ) {
	    QFileInfo fi( d->path );
	    if ( !fi.exists() )
		d->cleanPath = d->path;
	    else if ( fi.isDir() ) {
		QString dir = QDir::cleanDirPath( QDir( d->path ).canonicalPath() ) + "/";
		if ( dir == "//" )
		    d->cleanPath = "/";
		else
		    d->cleanPath = dir;
	    } else {
		QString p = QDir::cleanDirPath( fi.dir().canonicalPath() );
		d->cleanPath = p + "/" + fi.fileName();
	    }
	} else {
	    if ( d->path != "/" && d->path.right( 1 ) == "/" )
		d->cleanPath = QDir::cleanDirPath( d->path ) + "/";
	    else
		d->cleanPath = QDir::cleanDirPath( d->path );
	}

	if ( d->cleanPath.length() > 1 ) {
	    if ( d->cleanPath.left( 2 ) == "//" )
		d->cleanPath.remove( d->cleanPath.length() - 1, 1 );
	}
	d->cleanPathDirty = FALSE;
    }

    return d->cleanPath;
}

/*!
  Returns TRUE, if the URL is a local file, else
  it returns FALSE;
*/

bool QUrl::isLocalFile() const
{
    return d->protocol == "file";
}

/*!
  Returns the filename of the URL.
*/

QString QUrl::fileName() const
{
    if ( d->path.isEmpty() )
	return QString::null;

    return QFileInfo( d->path ).fileName();
}

/*!
  Adds the path \a pa to the path or the URL.
*/

void QUrl::addPath( const QString& pa )
{
    if ( pa.isEmpty() )
	return;

    QString p( pa );
    slashify( p );
    
    d->cleanPathDirty = TRUE;
    if ( d->path.isEmpty() ) {
	if ( p[ 0 ] != QChar( '/' ) )
	    d->path = "/" + p;
	else
	    d->path = p;
    } else {
	if ( p[ 0 ] != QChar( '/' ) && d->path.right( 1 ) != "/" )
	    d->path += "/" + p;
	else
	    d->path += p;
    }
}

/*!
  Returns the directory path of the URL.
*/

QString QUrl::dirPath() const
{
    if ( path().isEmpty() )
	return QString::null;

    return QFileInfo( path() ).dirPath() + "/";
}

/*!
  Encodes the string \a url.
*/

void QUrl::encode( QString& url )
{
    int oldlen = url.length();

    if ( !oldlen )
	return;

    QString newUrl;
    int newlen = 0;

    for ( int i = 0; i < oldlen ;++i ) {
	if ( QString( "<>#@\"&%$:,;?={}|^~[]\'`\\ \n\t\r" ).contains( url[ i ].unicode() ) ) {
	    newUrl[ newlen++ ] = QChar( '%' );

	    ushort c = url[ i ].unicode() / 16;
	    c += c > 9 ? 'A' - 10 : '0';
	    newUrl[ newlen++ ] = c;

	    c = url[ i ].unicode() % 16;
	    c += c > 9 ? 'A' - 10 : '0';
	    newUrl[ newlen++ ] = c;
	} else
	    newUrl[ newlen++ ] = url[ i ];
    }

    url = newUrl;
}

static ushort hex_to_int( ushort c )
{
    if ( c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if ( c >= '0' && c <= '9')
	return c - '0';
    return 0;
}

/*!
  Decodes the string \url.
*/

void QUrl::decode( QString& url )
{
    int oldlen = url.length();
    if ( !oldlen )
	return;

    int newlen = 0;

    QString newUrl;

    int i = 0;
    while ( i < oldlen ) {
	ushort c = url[ i++ ].unicode();
	if ( c == '%' ) {
	    c = hex_to_int( url[ i ].unicode() ) * 16 + hex_to_int( url[ i + 1 ].unicode() );
	    i += 2;
	}
	newUrl [ newlen++ ] = c;
    }

    url = newUrl;
}

/*!
  Composes a string of the URL and returns it. If \a encodedPath
  is TRUE, the path in the returned string will be encoded. If
  \a forcePrependProtocol is TRUE the file:/ protocol is also
  prepended if no remote network protocols are registered.
*/

QString QUrl::toString( bool encodedPath, bool forcePrependProtocol ) const
{
    QString res, p = path();
    if ( encodedPath )
	encode( p );

    if ( isLocalFile() ) {
	if ( !forcePrependProtocol && ( !qNetworkProtocolRegister ||
					qNetworkProtocolRegister->count() == 0 ||
					QNetworkProtocol::hasOnlyLocalFileSystem() ) )
	    res = p;
	else
	    res = d->protocol + ":" + p;
    } else {
	res = d->protocol + "://";
	if ( !d->user.isEmpty() || !d->pass.isEmpty() ) {
	    if ( !d->user.isEmpty() )
		res += d->user;
	    if ( !d->pass.isEmpty() )
		res += ":" + d->pass;
	    res += "@";
	}
	res += d->host;
	if ( d->port != -1 )
	    res += ":" + QString( "%1" ).arg( d->port );
	res += p;
    }

    if ( qNetworkProtocolRegister && qNetworkProtocolRegister->count() > 0 &&
	 !QNetworkProtocol::hasOnlyLocalFileSystem() ) {
	if ( !d->refEncoded.isEmpty() )
	    res += "#" + d->refEncoded;
	if ( !d->queryEncoded.isEmpty() )
	    res += "?" + d->queryEncoded;
    }

    return res;
}

/*!
  Composes a string of the URL and returns it.
*/

QUrl::operator QString() const
{
    return toString();
}

/*!
  Goes one directory up.
*/

bool QUrl::cdUp()
{
    d->path += "/..";
    d->cleanPathDirty = TRUE;
    return TRUE;
}

