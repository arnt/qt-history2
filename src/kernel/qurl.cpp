/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurl.cpp#40 $
**
** Implementation of QFileDialog class
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
#include "qurlinfo.h"
#include "qnetworkprotocol.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <qapplication.h>
#include <qmap.h>

struct QUrlPrivate
{
    QString protocol;
    QString user;
    QString pass;
    QString host;
    QString path;
    QString refEncoded;
    QString queryEncoded;
    bool isValid;
    int port;
    QString nameFilter;
    QDir dir;
    QMap<QString, QUrlInfo> entryMap;
    QNetworkProtocol *networkProtocol;
};

/*!
  \class QUrl qurl.h

  The QUrl class is provided for a easy working with URLs.
  It does all parsing, decoding, encoding and so on. Also
  methodes like listing directories, copying URLs, removing
  URLs, renaming URLs and some more are implemented. These
  funcktions work by default only for the local filesystem,
  for other network protocols, an implementation of the
  required protocol has to be registered. For more information
  about this, see the QNetworkProtocol documentation.

  Mention that URL has some restrictions regarding the path
  encoding. URL works intern with the decoded path and
  and encoded query. For example in

  http://localhost/cgi-bin/test%20me.pl?cmd=Hello%20you

  would result in a decoded path "/cgi-bin/test me.pl"
  and in the encoded query "cmd=Hello%20you".
  Since path is internally always encoded you may NOT use
  "%00" in the path while this is ok for the query.

  \sa QNetworkProtocol::QNetworkProtocol()
*/

/*!
  \fn void QUrl::entry( const QUrlInfo &i )

  This signal is emitted after listEntries() was called and
  a new entry (file) has been read from the list of files. \a i
  holds the information about the new etry.
*/

/*!
  \fn void QUrl::finished( int action )

  This signal is emitted when a data transfer of some sort finished.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrl::start( int action )

  This signal is emitted when a data transfer of some sort started.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrl::createdDirectory( const QUrlInfo &i )

  This signal is emitted when mkdir() has been succesful
  and the directory has been created. \a i holds the information
  about the new directory.
*/

/*!
  \fn void QUrl::removed( const QString &name )

  This signal is emitted when remove() has been succesful
  and the file has been removed. \a name is the filename
  of the removed file.
*/

/*!
  \fn void QUrl::itemChanged( const QString &oldname, const QString &newname )

  This signal is emitted whenever a file, which is a child of this URL,
  has been changed e.g. by successfully calling rename(). \a oldname is
  the original name of the file and \a newname is the name which the file
  go now.
*/

/*!
  \fn void QUrl::error( int ecode, const QString &msg )

  This signal is emitted whenever an error occures. \a ecode
  is the error code, and \a msg an error message which can be
  e.g. displayed to the user.

  \a ecode is one of
	ErrDeleteFile
	ErrRenameFile
	ErrCopyFile
	ErrReadDir
	ErrCreateDir
	ErrUnknownProtocol
	ErrParse
	ErrLoginIncorrect
	ErrHostNotFound
	ErrValid
*/

/*!
  \fn void QUrl::data( const QCString &data )

  This signal is emitted when new \a data has been received.
*/

/*!
  \fn void QUrl::putSuccessful( const QCString &data )

  This signal is emitted after successfully calling put(). \a data is the data
  which has been put.
*/

/*!
  \fn void QUrl::copyProgress( const QString &from, const QString &to, int step, int total )

  When copying a file this signal is emitted. \a from is the file which
  is copied, \a to the destination. \a step is the progress
  (always <= \a total) or -1, if copying just started. \a total is the
  number of steps needed to copy the file.

  This signal can be used to show the progress when copying files.
*/

/*!
  \fn void QUrl::emitEntry( const QUrlInfo & );

  Emits the signal entry( const QUrlInfo & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitFinished( int action )

  Emits the signal finished(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
  \a action can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrl::emitStart( int action )

  Emits the signal start(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
  \a action can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrl::emitCreatedDirectory( const QUrlInfo & )

  Emits the signal createdDirectory( const QUrlInfo & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitRemoved( const QString & )

  Emits the signal removed( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitItemChanged( const QString &oldname, const QString &newname )

  Emits the signal itemChanged( const QString &, const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitError( int ecode, const QString &msg )

  Emits the signal error( int, const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitData( const QCString &d )

  Emits the signal data( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitPutSuccessful( const QCString &d )

  Emits the signal putSuccessful( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/


/*!
  \fn void QUrl::emitCopyProgress( const QString &from, const QString &to, int step, int total )

  Emits the signal copyProgress(  const QString &, const QString &, int, int).
  This method is mainly provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  Constructs an empty, malformed URL.
*/

QUrl::QUrl()
{
    d = new QUrlPrivate;
    d->isValid = FALSE;
    d->nameFilter = "*";
    d->networkProtocol = 0;
    d->port = -1;
}

/*!
  Constructs and URL using \a url and parses this string.

  \a url is considered to be encoded. You can pass strings like
  "/home/weis", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Torben%20Weis" will be decoded to
  "/home/Torben Weis". This means: If you have a usual UNIX like
  path, you have to use \link encode first before you pass it to URL.
*/

QUrl::QUrl( const QString& url )
{
    d = new QUrlPrivate;
    d->protocol = "file";
    d->port = -1;
    d->nameFilter = "*";
    d->networkProtocol = 0;
    QString tmp = url.stripWhiteSpace();
    parse( tmp );
}

/*!
  Copy constructor.
*/

QUrl::QUrl( const QUrl& url )
    : QObject()
{
    d = new QUrlPrivate;
    *d = *url.d;
    getNetworkProtocol();
}

/*!
  Returns TRUE, if \a url is relative, else it returns FALSE.
*/

bool QUrl::isRelativeUrl( const QString &url )
{
    int colon = url.find( ":" );
    int slash = url.find( "/" );

    return ( colon == -1 || ( slash != -1 && colon > slash ) );
}

/*!
  Constructs and URL taking \a url as base and \a relUrl_ as
  relative URL to \a url.
*/

QUrl::QUrl( const QUrl& url, const QString& relUrl_ )
{
  d = new QUrlPrivate;
  QString relUrl = relUrl_.stripWhiteSpace();
  getNetworkProtocol();

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
	  d->path = p;
      }
  }
}

/*!
  Destructor.
*/

QUrl::~QUrl()
{
    if ( d->networkProtocol )
 	delete d->networkProtocol;
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
  Returns TRUE, of the URL contains an username,
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
  Returns TRUE, of the URL contains an password,
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
  Returns TRUE, of the URL contains an hostname,
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
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
  Returns TRUE, of the URL contains a path,
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
  Resets all values if the URL to its default values.
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
    if ( d->networkProtocol )
 	delete d->networkProtocol;
    d->networkProtocol = 0;
}

/*!
  Parses the \a url.
*/

void QUrl::parse( const QString& url )
{
    if ( url.isEmpty() ) {
	d->isValid = FALSE;
	return;
    }

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
    if ( url.find( ":/" ) == -1 ) {
	table[ 0 ][ 1 ] = Path;
	relPath = TRUE;
    } else
	table[ 0 ][ 1 ] = Protocol;
	
    int state = Init; // parse state
    int input; // input token
    bool hasAt = url.find( "@" ) != -1;

    QString buffer;
    QChar c = url[ 0 ];
    int i = 0;

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

	// #### hack: don't know how to make this better...
	if ( ( state == Pass || state == User ) && !hasAt ) {
	    QString p = d->protocol;
	    if ( p.isEmpty() )
		p = "file";
	    if ( p == "file" )
		state = Path;
	    else
		state = Host;
	}
	
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
	case Port: {
	    if ( d->port == -1 )
		d->port = 0;
	    QString p;
	    p.setNum( d->port );
	    p += c;
	    d->port = p.toInt();
	} break;
	default:
	    break;
	}

	++i;
	if ( i > (int)url.length() - 1 || state == Done || state == 0 )
	    break;
	c = url[ i ];
	
    }

    // error
    if ( i < (int)url.length() - 1 ) {
	emit error( ErrParse, QUrl::tr( "Error in parsing `%1'" ).arg( url ) );
	d->isValid = FALSE;
	return;
    }
	
    if ( d->protocol.isEmpty() )
	d->protocol = oldProtocol;

    if ( d->path.isEmpty() )
	d->path = "/";

    // #### do some corrections, should be done nicer too
    if ( !d->pass.isEmpty() && d->pass[ 0 ] == ':' )
	d->pass.remove( 0, 1 );
    if ( !d->path.isEmpty() ) {
	if ( d->path[ 0 ] == '@' || d->path[ 0 ] == ':' )
	    d->path.remove( 0, 1 );
	if ( d->path[ 0 ] != '/' && !relPath )
	    d->path.prepend( "/" );
    }
    if ( !d->refEncoded.isEmpty() && d->refEncoded[ 0 ] == '#' )
	d->refEncoded.remove( 0, 1 );
    if ( !d->queryEncoded.isEmpty() && d->queryEncoded[ 0 ] == '?' )
	d->queryEncoded.remove( 0, 1 );
    if ( !d->host.isEmpty() && d->host[ 0 ] == '@' )
	d->host.remove( 0, 1 );

    decode( d->path );

    if ( d->networkProtocol )
 	delete d->networkProtocol;
    getNetworkProtocol();

#if 0
    qDebug( "URL: %s", url.latin1() );
    qDebug( "protocol: %s", d->protocol.latin1() );
    qDebug( "user: %s", d->user.latin1() );
    qDebug( "pass: %s", d->pass.latin1() );
    qDebug( "host: %s", d->host.latin1() );
    qDebug( "path: %s", path().latin1() );
    qDebug( "ref: %s", d->refEncoded.latin1() );
    qDebug( "query: %s", d->queryEncoded.latin1() );
    qDebug( "port: %d\n", d->port );
#endif
}

/*!
  Assign operator.

  \a url is considered to be encoded. You can pass strings like
  "/home/weis", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Torben%20Weis" will be decoded to
  "/home/Torben Weis". This means: If you have a usual UNIX like
  path, you have to use \link encode first before you pass it to URL.
*/

QUrl& QUrl::operator=( const QString& url )
{
    reset();
    parse( url );

    return *this;
}

/*!
  Assign operator.
*/

QUrl& QUrl::operator=( const QUrl& url )
{
    *d = *url.d;
    getNetworkProtocol();
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
  Compares this URL with \a url.
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
    QString fn = name;

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
    int pos = path.find( '?' );
    if ( pos == -1 ) {
	d->path = path;
	d->queryEncoded = "";
    } else {
	d->path = path.left( pos );
	d->queryEncoded = path.mid( pos + 1 );
    }

    decode( d->path );
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
  Returns the path of the URL.
*/

QString QUrl::path() const
{
    QString res;
    if ( isLocalFile() ) {
	QFileInfo fi( d->path );
	if ( !fi.exists() )
	    res = d->path;
	else if ( fi.isDir() ) {
	    QString dir = QDir::cleanDirPath( QDir( d->path ).canonicalPath() ) + "/";
	    if ( dir == "//" )
		res = "/";
	    else
		res = dir;
	} else {
	    QString p = QDir::cleanDirPath( fi.dir().canonicalPath() );
	    res = p + "/" + fi.fileName();
	}
    } else {
	if ( d->path != "/" && d->path.right( 1 ) == "/" )
	    res = QDir::cleanDirPath( d->path ) + "/";
	else
	    res = QDir::cleanDirPath( d->path );
    }

    if ( res.length() > 1 ) {
	if ( res.left( 2 ) == "//" )
	    res.remove( res.length() - 1, 1 );
    }

    return res;
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
  Adds the path \a p to the path or the URL.
*/

void QUrl::addPath( const QString& p )
{
    if ( p.isEmpty() )
	return;

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

    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
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
  Encodes the strung \a url.
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

/*!
*/

static ushort hex2int( ushort c )
{
    if ( c >= 'A' && c <='F')
	return c - 'A' + 10;
    if ( c >= 'a' && c <='f')
	return c - 'a' + 10;
    if ( c >= '0' && c <='9')
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
	    c = hex2int( url[ i ].unicode() ) * 16 + hex2int( url[ i + 1 ].unicode() );
	    i += 2;
	}
	newUrl [ newlen++ ] = c;
    }

    url = newUrl;
}

/*!
  Starts listing a directory. The signal start() is emitted, before the
  first entry is listed, and after the last one finished() is emitted.
  If an error occures, the signal error() with an error code and an error
  message is emitted.

  You can rely on the parameters \nameFilter \a filterSpec and \a sortSpec
  only when working on the local filesystem, this may not be supported when
  using a network protocol.
*/

void QUrl::listEntries()
{
    if ( !checkValid() )
	return;

    clearEntries();
    if ( isLocalFile() ) {
	d->dir = QDir( d->path );
	d->dir.setNameFilter( d->nameFilter );
	d->dir.setMatchAllDirs( TRUE );
	if ( !d->dir.isReadable() ) {
	    QString msg = QUrl::tr( "Could not read directory\n" + d->path );
	    emit error( ErrReadDir, msg );
	    return;
	}
	
	const QFileInfoList *filist = d->dir.entryInfoList( QDir::All | QDir::Hidden );
	if ( !filist ) {
	    QString msg = QUrl::tr( "Could not read directory\n" + d->path );
	    emit error( ErrReadDir, msg );
	    return;
	}
	
	emit start( ActListDirectory );
	QFileInfoListIterator it( *filist );
	QFileInfo *fi;
	while ( ( fi = it.current()) != 0 ) {
	    ++it;
	    QUrlInfo inf( fi->fileName(), 0/*permissions*/, fi->owner(), fi->group(),
			  fi->size(), fi->lastModified(), fi->lastRead(), fi->isDir(), fi->isFile(),
			  fi->isSymLink(), fi->isWritable(), fi->isReadable(), fi->isExecutable() );
	    emit entry( inf );
	    addEntry( inf );
	}
	emit finished( ActListDirectory );
    } else if ( d->networkProtocol &&
		d->networkProtocol->supportedOperations() & QNetworkProtocol::OpListEntries ) {
	emit start( ActListDirectory );
	setNameFilter( d->nameFilter );
	d->networkProtocol->listEntries();
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support listing directores" ).
		    arg( d->protocol ).arg( d->protocol ) );
}

/*!
  Tries to create a directory with the name \a dirname.
  If it has been successful an entry() signal with the
  new entry is emitted and the createdDirectory() with
  the information about the new entry is emitted too.
  Else the error() signal is emitted.
*/

void QUrl::mkdir( const QString &dirname )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	d->dir = QDir( d->path );
	if ( d->dir.mkdir( dirname ) ) {
	    QFileInfo fi( d->dir, dirname );
	    QUrlInfo inf( fi.fileName(), 0/*permissions*/, fi.owner(), fi.group(),
			  fi.size(), fi.lastModified(), fi.lastRead(), fi.isDir(), fi.isFile(),
			  fi.isSymLink(), fi.isWritable(), fi.isReadable(), fi.isExecutable() );
	    emit entry( inf );
	    emit createdDirectory( inf );
	} else {
	    QString msg = QUrl::tr( "Could not create directory\n" + dirname );
	    emit error( ErrCreateDir, msg );
	}
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpMkdir ) {
	d->networkProtocol->mkdir( dirname );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support making directores" ).
		    arg( d->protocol ).arg( d->protocol ) );
}

/*!
  Tries to remove the file \a filename.
  If it has been successful the signal removed() with
  the \a filename is emitted.
  Else the error() signal is emitted.
*/

void QUrl::remove( const QString &filename )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	QDir dir( d->path );
	if ( dir.remove( filename ) )
	    emit removed( filename );
	else {
	    QString msg = QUrl::tr( "Could not delete file\n" + filename );
	    emit error( ErrDeleteFile, msg );
	}
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRemove ) {
	d->networkProtocol->remove( filename );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support removing files" ).
		    arg( d->protocol ).arg( d->protocol ) );
}

/*!
  Tries to rename the file \a oldname by \a newname.
  If it has been successful the signal itemChanged() with
  the old and new name of the file is emitted.
  Else the error() signal is emitted.
*/

void QUrl::rename( const QString &oldname, const QString &newname )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	QDir dir( d->path );
	if ( dir.rename( oldname, newname ) )
	    emit itemChanged( oldname, newname );
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRename ) {
	d->networkProtocol->rename( oldname, newname );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support renaming files" ).
		    arg( d->protocol ).arg( d->protocol ) );
}

/*!
  Copies the file \a from to \a to on the local filesystem.
*/

void QUrl::copy( const QString &from, const QString &to )
{
    QFile f( from );
    if ( !f.open( IO_ReadOnly ) )
	return;

    QFileInfo fi( to );
    if ( fi.exists() ) {
	if ( !fi.isFile() ) {
	    f.close();
	    qWarning( QString( "Couldn't write file\n%1" ).arg( to ) );
	    return;
	}
    }

    QFile f2( to );
    if ( !f2.open( IO_WriteOnly ) ) {
	f.close();
	return;
    }

    char *block = new char[ 1024 ];
    bool error = FALSE;
    int sum = 0;
    emit copyProgress( from, to, -1, 100 );
    while ( !f.atEnd() ) {
	int len = f.readBlock( block, 100 );
	if ( len == -1 ) {
	    error = TRUE;
	    break;
	}
	sum += len;
	emit copyProgress( from, to, ( sum * 100 ) / f.size(), 100 );
	f2.writeBlock( block, len );
	qApp->processEvents();
    }

    delete[] block;

    f.close();
    f2.close();

    return;
}

/*!
  Copies \a files to the directory \a dest. If \a move is TRUE,
  the files are moved and not copied.
*/

void QUrl::copy( const QStringList &files, const QString &dest, bool move )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	emit start( move ? ActMoveFiles : ActCopyFiles );
	QString de = dest;
	if ( de.left( QString( "file:" ).length() ) == "file:" )
	    de.remove( 0, QString( "file:" ).length() );
	QStringList::ConstIterator it = files.begin();
	for ( ; it != files.end(); ++it ) {
	    if ( QFileInfo( *it ).isFile() ) {
		copy( *it, de + "/" + QFileInfo( *it ).fileName() );
		if ( move )
		    QFile::remove( *it );
	    }
	}
	emit finished( move ? ActMoveFiles : ActCopyFiles );
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpCopy ) {
	d->networkProtocol->copy( files, dest, move );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support copying files" ).
		    arg( d->protocol ).arg( d->protocol ) );
}

/*!
  Returns TRUE if the url is a directory, else
  returns FALSE.
*/

bool QUrl::isDir()
{
    if ( isLocalFile() ) {
	if ( QFileInfo( path() ).isDir() )
	    return TRUE;
	else
	    return FALSE;
    }

    if ( d->entryMap.contains( "." ) )
	return d->entryMap[ "." ].isDir();
    else {
	if ( d->networkProtocol &&
	     ( d->networkProtocol->supportedOperations() & QNetworkProtocol::OpUrlIsDir ) )
	    return d->networkProtocol->isUrlDir();
	// if we are here, we really have a problem!!
	// Checking for a trailing slash to find out
	// if URL is a dir is not reliable at all :-)
	if ( !d->path.isEmpty() )
	    return d->path.right( 1 ) == "/";
    }

    return FALSE;
}

/*!
 */

void QUrl::put( const QCString &data )
{
    if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpPut )
	d->networkProtocol->put( data );
}

/*!
  Sets the name filter of the URL

  \sa QDir::setNameFilter()
*/

void QUrl::setNameFilter( const QString &nameFilter )
{
    d->nameFilter = nameFilter;
}

/*!
  Returns the name filter of the URL

  \sa QUrl::setNameFilter() QDir::nameFilter()
*/

QString QUrl::nameFilter() const
{
    return d->nameFilter;
}

/*!
  Composes a string of the URL and returns it. If \a encodedPath
  is TRUE, the path in the returned string will be encoded. If
  \a forcePrependProtocol is TRUE the file:/ protocol is also
  prepended if no network protocols are reguistered.
*/

QString QUrl::toString( bool encodedPath, bool forcePrependProtocol ) const
{
    QString res, p = path();
    if ( encodedPath )
	encode( p );

    if ( isLocalFile() ) {
	if ( !forcePrependProtocol && ( !qNetworkProtocolRegister || ( qNetworkProtocolRegister &&
								       qNetworkProtocolRegister->count() == 0 ) ) )
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

    if ( qNetworkProtocolRegister && qNetworkProtocolRegister->count() > 0 ) {
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
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return TRUE;
}

/*!
  Clears the cache of file entries.
*/

void QUrl::clearEntries()
{
    d->entryMap.clear();
}

/*!
  Adds an entry to the file entry cache.
*/

void QUrl::addEntry( const QUrlInfo &i )
{
    d->entryMap[ i.name().stripWhiteSpace() ] = i;
}

/*!
  Returns the URL information for the file entry \a entry.
*/

QUrlInfo QUrl::info( const QString &entry ) const
{
    if ( d->entryMap.contains( entry ) ) {
	return d->entryMap[ entry ];
    } else {
	return QUrlInfo();
    }
}

/*!
  Find a network protocol for the URL.
*/

void QUrl::getNetworkProtocol()
{
    if ( isLocalFile() || !qNetworkProtocolRegister ) {
	d->networkProtocol = 0;
	return;
    }

    QNetworkProtocol *p = QNetworkProtocol::getNetworkProtocol( d->protocol );
    if ( !p ) {
	d->networkProtocol = 0;
	return;
    }

    d->networkProtocol = (QNetworkProtocol *)p;
    d->networkProtocol->setUrl( this );
}

/*!
 */

bool QUrl::checkValid()
{
    if ( !d->isValid ) {
	emit error( ErrValid, QUrl::tr( "The entered URL is not valid!" ) );
	return FALSE;
    } else
	return TRUE;
}

/*
 */
void QUrl::deleteNetworkProtocol()
{
    if ( d->networkProtocol )
	delete d->networkProtocol;
    d->networkProtocol = 0;
}
