/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurl.cpp#14 $
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
#include "qnetprotocol.h"

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
    bool isMalformed;
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
	ActGet
	ActPut
*/

/*!
  \fn void QUrl::start( int action )

  This signal is emitted when a data transfer of some sort started.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActGet
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
	ErrParseError
*/

/*!
  \fn void QUrl::data( const QString &data )

  This signal is emitted when new \a data has been received after e.g. calling get().
*/

/*!
  \fn void QUrl::putSuccessful( const QString &data )

  This signal is emitted after successfully calling put(). \a data is the data
  which has been put.
*/

/*!
  \fn void QUrl::urlIsDir()

  When calling isFile() or isDir() and the URL is a dir, this signal
  is emitted.
*/

/*!
  \fn void QUrl::urlIsFile()

  When calling isFile() or isDir() and the URL is a file, this signal
  is emitted.
*/

/*!
  \fn void QUrl::emitEntry( const QUrlInfo & );

  Emits the signal entry( const QUrlInfo & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitFinished()

  Emits the signal finished(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitStart()

  Emits the signal start(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
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
  \fn void QUrl::emitData( const QString &d )

  Emits the signal data( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitUrlIsDir()

  Emits the signal urlIsDir(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitUrlIsFile()

  Emits the signal urlIsFile(). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrl::emitPutSuccessful( const QString &d )

  Emits the signal putSuccessful( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/


/*!
  Constructs an empty, malformed URL.
*/

QUrl::QUrl()
{
    d = new QUrlPrivate;
    d->isMalformed = TRUE;
    d->nameFilter = "*";
    d->networkProtocol = 0;
}

/*!
  Constructs and URL using \a url and parses this string.

  \a url is considered to be encoded. You can pass strings like
  "/home/weis", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Torben%20Weis" will be decoded to
  "/home/Torben Weis". This means: If you have a usual UNIX like
  path, you have to use @ref encode first before you pass it to URL.
*/

QUrl::QUrl( const QString& url )
{
    d = new QUrlPrivate;
    // Assume the default protocol
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
  Constructs and URL for the file \a relUrl_ in the directory
  \a url.
*/

QUrl::QUrl( const QUrl& url, const QString& relUrl_ )
{
    d = new QUrlPrivate;
    QString relUrl = relUrl_.stripWhiteSpace();
    getNetworkProtocol();

    // relUrl starts in the root ?
    if ( relUrl[0] == '/' ) {
	*this = url;
 	if ( QFileInfo( d->path ).isFile() )
 	    d->path = QFileInfo( d->path ).dirPath();
	int pos = relUrl.find( '#' );
	QString tmp;
	if ( pos == -1 ) {
	    tmp = relUrl;
	    setRef( "" );
	} else {
	    setRef( relUrl.mid( pos + 1 ) );
	    tmp = relUrl.left( pos );
	}
	setEncodedPathAndQuery( tmp );
    } else if ( relUrl[0] == '#' ) {
	// relUrl just affects the reference ?
	*this = url;
 	if ( QFileInfo( d->path ).isFile() )
 	    d->path = QFileInfo( d->path ).dirPath();
	setRef( relUrl.mid(1) );
    } else if ( relUrl.find( ":/" ) != -1 ) {
	// relUrl is a complete QUrl ?
	*this = relUrl;
    } else {	
	*this = url;
 	if ( QFileInfo( d->path ).isFile() )
 	    d->path = QFileInfo( d->path ).dirPath();
	int pos = relUrl.find( '#' );
	QString tmp;
	if ( pos == -1 ) {
	    tmp = relUrl;
	    setRef( "" );
	} else {
	    setRef( relUrl.mid( pos + 1 ) );
	    tmp = relUrl.left( pos );
	}
	decode( tmp );
	setFileName( tmp );
    }
    d->nameFilter = "*";
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
  #### todo
*/

void QUrl::setQuery( const QString& txt )
{
    d->queryEncoded = txt;
}

/*!
  #### todo
*/

QString QUrl::query() const
{ 	
    return d->queryEncoded;
}

/*!
  #### todo
*/

QString QUrl::ref() const
{
    return d->refEncoded;
}

/*!
  #### todo
*/

void QUrl::setRef( const QString& txt )
{
    d->refEncoded = txt;
}

/*!
  #### todo
*/

bool QUrl::hasRef() const
{
    return !d->refEncoded.isEmpty();
}

/*!
  #### todo
*/

bool QUrl::isMalformed() const
{
    return d->isMalformed;
}

/*!
  #### todo
*/

void QUrl::reset()
{
    // Assume the default protocol.
    d->protocol = "file";
    d->user = "";
    d->pass = "";
    d->host = "";
    d->path = "";
    d->queryEncoded = "";
    d->refEncoded = "";
    d->isMalformed = FALSE;
    d->port = -1;
    if ( d->networkProtocol )
 	delete d->networkProtocol;
    d->networkProtocol = 0;
}

/*!
  #### todo
*/

void QUrl::parse( const QString& url )
{
    if ( url.isEmpty() ) {
	d->isMalformed = true;
	return;
    }

    d->isMalformed = false;

    QString port;
    int start = 0;
    uint len = url.length();
    QChar* buf = new QChar[ len + 1 ];
    QChar* orig = buf;
    memcpy( buf, url.unicode(), len * sizeof( QChar ) );

    uint pos = 0;

    // Node 1: Accept alpha or slash
    QChar x = buf[pos++];
    if ( x == '/' )
	goto Node9;
    if ( !isalpha( (int)x ) )
	goto NodeErr;

    // Node 2: Accept any amount of alphas
    // Proceed with :// :/ or :
    while( isalpha( buf[pos] ) && pos < len ) pos++;
    if ( pos == len )
	goto NodeErr;
    if (buf[pos] == ':' && buf[pos+1] == '/' && buf[pos+2] == '/' ) {
	d->protocol = QString( orig, pos ).lower();
	pos += 3;
    } else if (buf[pos] == ':' && buf[pos+1] == '/' ) {
	d->protocol = QString( orig, pos ).lower();
	pos++;
	start = pos;
	goto Node9;
    } else if ( buf[pos] == ':' ) {
	d->protocol = QString( orig, pos ).lower();
	pos++;
	goto Node11;
    } else
	goto NodeErr;

    //Node 3: We need at least one character here
    if ( pos == len )
	goto NodeOk;
    //    goto NodeErr;
#if 0
    start = pos++;
#else
    start = pos;
#endif

    // Node 4: Accept any amount of characters.
    // Terminate or / or @
    while( buf[pos] != ':' && buf[pos] != '@' && buf[pos] != '/' && pos < len ) pos++;
    if ( pos == len ) {
	d->host = QString( buf + start, pos - start );
	goto NodeOk;
    }

    x = buf[pos];
    if ( x == '@' ) {
	d->user = QString( buf + start, pos - start );
	pos++;
	goto Node7;
    } else if ( x == '/' ) {
	d->host = QString( buf + start, pos - start );
	start = pos++;
	goto Node9;
    } else if ( x != ':' )
	goto NodeErr;
    d->user = QString( buf + start, pos - start );
    pos++;

    // Node 5: We need at least one character
    if ( pos == len )
	goto NodeErr;
    start = pos++;

    // Node 6: Read everything until @
    while( buf[pos] != '@' && pos < len ) pos++;
    if ( pos == len ) {
	// Ok the : was used to separate host and port
	d->host = d->user;
	d->user = "";
	QString tmp( buf + start, pos - start );
	char *endptr;
	d->port = (unsigned short int)strtol(tmp.ascii(), &endptr, 10);
	if ((pos == len) && (strlen(endptr) == 0))
	    goto NodeOk;
	// there is more after the digits
	pos -= strlen(endptr);
	start = pos++;
	goto Node9;
    }
    d->pass = QString( buf + start, pos - start);
    pos++;

    // Node 7: We need at least one character
Node7:
    if ( pos == len )
	goto NodeErr;
    start = pos++;

    // Node 8: Read everything until / : or terminate
    while( buf[pos] != '/' && buf[pos] != ':' && pos < len ) pos++;
    if ( pos == len ) {
	d->host = QString( buf + start, pos - start );
	goto NodeOk;
    }

    x = buf[pos];
    d->host = QString( buf + start, pos - start );
    if ( x == '/' ) {
	start = pos++;
	goto Node9;
    } else if ( x != ':' )
	goto NodeErr;
    pos++;

    // Node 8a: Accept at least one digit
    if ( pos == len )
	goto NodeErr;
    start = pos;
    if ( !isdigit( buf[pos++] ) )
	goto NodeErr;

    // Node 8b: Accept any amount of digits
    while( isdigit( buf[pos] ) && pos < len ) pos++;
    port = QString( buf + start, pos - start );
    d->port = port.toUShort();
    if ( pos == len )
	goto NodeOk;
    start = pos++;

    // Node 9: Accept any character and # or terminate
Node9:
    while( buf[pos] != '#' && pos < len ) pos++;
    if ( pos == len ) {
	QString tmp( buf + start, len - start );
	setEncodedPathAndQuery( tmp );
	// setEncodedPathAndQuery( QString( buf + start, pos - start ) );
	goto NodeOk;
    }
    else if ( buf[pos] != '#' )
	goto NodeErr;
    setEncodedPathAndQuery( QString( buf + start, pos - start ) );
    pos++;

    // Node 10: Accept all the rest
    d->refEncoded = QString( buf + pos, len - pos );
    goto NodeOk;

    // Node 11 We need at least one character
Node11:
    start = pos;
    if ( pos++ == len )
	goto NodeOk;
    //    goto NodeErr;

    // Node 12: Accept the res
    setEncodedPathAndQuery( QString( buf + start, len - start ) );
    goto NodeOk;

NodeOk:
    if ( d->path.isEmpty() )
	d->path = "/";
    delete []orig;
    if ( d->networkProtocol )
 	delete d->networkProtocol;
    if ( d->port == -1 ) {
	if ( d->protocol == "ftp" )
	    d->port = 21;
	else if ( d->protocol == "http" )
	    d->port = 80;
    }
    getNetworkProtocol();

    return;

NodeErr:
    if ( d->path.isEmpty() )
	d->path = "/";
    qWarning( "Error in parsing \"%s\"", url.ascii() );
    emit error( ErrParse, QUrl::tr( "Error in parsing `%1'" ).arg( url ) );
    delete []orig;
    d->isMalformed = true;

}

/*!
  #### todo

  \a url is considered to be encoded. You can pass strings like
  "/home/weis", in this case the protocol "file" is assumed.
  This is dangerous since even this simple path is assumed to be
  encoded. For example "/home/Torben%20Weis" will be decoded to
  "/home/Torben Weis". This means: If you have a usual UNIX like
  path, you have to use @ref encode first before you pass it to URL.
*/

QUrl& QUrl::operator=( const QString& url )
{
    reset();
    parse( url );

    return *this;
}

/*!
  #### todo
*/

QUrl& QUrl::operator=( const QUrl& url )
{
    *d = *url.d;
    getNetworkProtocol();
    return *this;
}

/*!
  #### todo
*/

bool QUrl::operator==( const QUrl& url ) const
{
    if ( isMalformed() || url.isMalformed() )
	return FALSE;

    if ( d->protocol == url.d->protocol &&
	 d->user == url.d->user &&
	 d->pass == url.d->pass &&
	 d->host == url.d->host &&
	 d->path == url.d->path &&
	 d->queryEncoded == url.d->queryEncoded &&
	 d->refEncoded == url.d->refEncoded &&
	 d->isMalformed == url.d->isMalformed &&
	 d->port == url.d->port )
	return TRUE;

    return FALSE;
}

/*!
  #### todo
*/

bool QUrl::operator==( const QString& url ) const
{
    QUrl u( url );
    return ( *this == u );
}

/*!
  #### todo
*/

bool QUrl::cmp( QUrl &url, bool ignoreTrailing )
{
    if ( ignoreTrailing ) {
	QString path1 = path(1);
	QString path2 = url.path(1);
	if ( path1 != path2 )
	    return FALSE;

	if ( d->protocol == url.d->protocol &&
	     d->user == url.d->user &&
	     d->pass == url.d->pass &&
	     d->host == url.d->host &&
	     d->path == url.d->path &&
	     d->queryEncoded == url.d->queryEncoded &&
	     d->refEncoded == url.d->refEncoded &&
	     d->isMalformed == url.d->isMalformed &&
	     d->port == url.d->port )
	    return TRUE;
	
	return FALSE;
    }

    return ( *this == url );
}

/*!
  #### todo
*/

void QUrl::setFileName( const QString& name )
{
    // Remove '/' in the front
    int start = 0;
    while( name[start] == '/' )
	start++;

    // Empty path ?
    int len = d->path.length();
    if ( len == 0 ) {
	d->path = "/";
	d->path += name.mid( start );
	if ( d->networkProtocol )
	    d->networkProtocol->setUrl( this );
	return;
    }

    // The current path is a directory ?
    if ( d->path[ len - 1 ] == '/' ) {
	// See wether there are multiple '/' characters
	while ( len >= 1 && d->path[ len - 1 ] == '/' )
	    len--;

	// Does the path only consist of '/' characters ?
	if ( len == 0 && d->path[ 1 ] == '/' ) {
	    d->path = "/";
	    d->path += name.mid( start );
	    if ( d->networkProtocol )
		d->networkProtocol->setUrl( this );
	    return;
	}

	// Just append the filename
	d->path += name.mid( start );
	if ( d->networkProtocol )
	    d->networkProtocol->setUrl( this );
	return;
    }

    // Find the rightmost '/'
    int i = d->path.findRev( '/', len - 1 );
    // If ( i == -1 ) => The first character is not a '/' ???
    // This looks strange ...
    if ( i == -1 ) {
	d->path = "/";
	d->path += name.mid( start );
	if ( d->networkProtocol )
	    d->networkProtocol->setUrl( this );
	return;
    }

    // #### these two lines are not correct!
    //d->path.truncate( i + 1 );
    d->path += "/" + name.mid( start );

    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
  #### todo
*/

QString QUrl::encodedPathAndQuery( int trailing, bool noEmptyPath )
{
    QString tmp = path( trailing );
    if ( noEmptyPath && tmp.isEmpty() )
	tmp = "/";

    encode( tmp );

    if ( !d->queryEncoded.isEmpty() ) {
	tmp += "?";
	tmp += d->queryEncoded;
    }

    return tmp;
}

/*!
  #### todo
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
  #### todo
*/

QString QUrl::path() const
{
    return QDir::cleanDirPath( d->path );
}

/*!
  #### todo
*/

QString QUrl::path( int trailing ) const
{
    QString result = path();

    // No modifications to the trailing stuff ?
    if ( trailing == 0 )
	return result;
    else if ( trailing == 1 ) {
	int len = result.length();
	// Path is empty ? => It should still be empty
	if ( len == 0 )
	    result = "";
	// Add a trailing '/'
	else if ( result[ len - 1 ] != '/' )
	    result += "/";
	return result;
    } else if ( trailing == -1 ) {
	// Dont change anything if path is just the slash
	if ( result == "/" )
	    return result;
	int len = result.length();
	// Strip trailing slash
	if ( len > 0 && result[ len - 1 ] == '/' )
	    result.truncate( len - 1 );
	return result;
    } else
	assert( 0 );

    return QString::null;
}

/*!
  #### todo
*/

bool QUrl::isLocalFile() const
{
    return d->protocol == "file";
}

/*!
  #### todo
*/

QString QUrl::url()
{
    QString result = url( 0 );
    return result;
}

/*!
  #### todo
*/

QString QUrl::url( int trailing, bool stripRef )
{
    QString u = d->protocol;
    if ( hasHost() ) {
	u += "://";
	if ( hasUser() ) {
	    u += d->user;
	    if ( hasPass() ) {
		u += ":";
		u += d->pass;
	    }
	    u += "@";
	}
	u += d->host;
	if ( d->port != -1 ) {
	    QString buffer = QString( ":%1" ).arg( d->port );
	    u += buffer;
	}
    } else
	u += ":";

    QString tmp;
    if ( trailing == 0 )
	tmp = d->path;
    else
	tmp = path( trailing );
    encode( tmp );
    u += tmp;

    if ( !d->queryEncoded.isEmpty() ) {
	u += "?";
	u += d->queryEncoded;
    }

    if ( hasRef() && !stripRef ) {
	u += "#";
	u += d->refEncoded;
    }

    return u;
}

/*!
  #### todo
*/

QString QUrl::filename( bool stripTrailingSlash )
{
    QString fname;

    int len = d->path.length();
    if ( len == 0 )
	return fname;

    if ( stripTrailingSlash ) {
	while ( len >= 1 && d->path[ len - 1 ] == '/' )
	    len--;
    } else if ( d->path[ len - 1 ] == '/' ) {
	// Path is a directory => empty filename
	return fname;
    }

    // Does the path only consist of '/' characters ?
    if ( len == 1 && d->path[ 1 ] == '/' )
	return fname;

    int i = d->path.findRev( '/', len - 1 );
    // If ( i == -1 ) => The first character is not a '/' ???
    // This looks like an error to me.
    if ( i == -1 )
	return fname;

    fname = d->path.mid( i + 1 );
    return fname;
}

/*!
  #### todo
*/

void QUrl::addPath( const QString& txt )
{
    if ( txt.isEmpty() )
	return;

    int len = d->path.length();
    // Add the trailing '/' if it is missing
    if ( txt[ 0 ] != '/' && ( len == 0 || d->path[ len - 1 ] != '/' ) )
	d->path += "/";

    // No double '/' characters
    int start = 0;
    if ( len != 0 && d->path[ len - 1 ] == '/' )
	while( txt[ start ] == '/' )
	    start++;

    d->path += txt.mid( start );
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
  #### todo
*/

QString QUrl::directory( bool stripTrailingSlashFromResult,
			bool ignoreTrailingSlashInPath )
{
    QString result;
    // Have to deal with the traling slash before
    // we parse the string ?
    if ( ignoreTrailingSlashInPath )
	result = path( -1 );
    else
	result = d->path;

    // Check trivial cases
    if ( result.isEmpty() || result == "/" )
	return result;

    // Look out for the rightmost slash
    int i = result.findRev( "/" );
    // No slash => empty directory
    if ( i == -1 )
	return result;

    // Another trivial case
    if ( i == 0 ) {
	result = "/";
	return result;
    }

    // Strip th trailing slash from the result
    if ( stripTrailingSlashFromResult )
	result = d->path.left( i );
    else
	result = d->path.left( i + 1 );

    return result;
}

/*!
  #### todo
*/

void QUrl::encode( QString& url )
{
    int old_length = url.length();

    if ( !old_length )
	return;

    QString new_url;
    //char *new_url = new char[ old_length * 3 + 1 ];
    int new_length = 0;

    for ( int i = 0; i < old_length; i++ ) {
	// 'unsave' and 'reserved' characters
	// according to RFC 1738,
	// 2.2. QUrl Character Encoding Issues (pp. 3-4)
	// Torben: Added the space characters
	if ( strchr("<>#@\"&%$:,;?={}|^~[]\'`\\ \n\t\r", url[ i ].unicode() ) ) {
	    new_url[ new_length++ ] = '%';

	    char c = url[ i ].unicode() / 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;

	    c = url[ i ].unicode() % 16;
	    c += (c > 9) ? ('A' - 10) : '0';
	    new_url[ new_length++ ] = c;
	
	}
	else
	    new_url[ new_length++ ] = url[ i ];
    }

    //new_url[ new_length ] = 0;
    url = new_url;
}

/*!
  #### todo
*/

char QUrl::hex2int( char c )
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
  #### todo
*/

void QUrl::decode( QString& url )
{
    int old_length = url.length();
    if ( !old_length )
	return;

    int new_length = 0;

    // make a copy of the old one
    //char *new_url = new char[ old_length + 1];
    QString new_url;

    int i = 0;
    while( i < old_length ) {
	char character = url[ i++ ].unicode();
	if ( character == '%' ) {
	    character = hex2int( url[ i ].unicode() ) * 16 + hex2int( url[ i+1].unicode() );
	    i += 2;
	}
	new_url [ new_length++ ] = character;
    }
    //new_url [ new_length ] = 0;
    url = new_url;
}

/*!
  Starts listing a directory. The signal start() is emitted, before the
  first entry is listed, and after the last one finished() is emitted.
  If an error occures, the signal error() with an error code and an error
  message is emitted.

  You can rely on the parameters \a filterSpec and \a sortSpec only when
  working on the local filesystem, this may not be supported when using
  a newtwork protocol.
*/

void QUrl::listEntries( int filterSpec = QDir::DefaultFilter,
			int sortSpec   = QDir::DefaultSort )
{
    listEntries( d->nameFilter, filterSpec, sortSpec );
}

/*!
  Starts listing a directory. The signal start() is emitted, before the
  first entry is listed, and after the last one finished() is emitted.
  If an error occures, the signal error() with an error code and an error
  message is emitted.

  You can rely on the parameters \nameFilter \a filterSpec and \a sortSpec
  only when working on the local filesystem, this may not be supported when
  using a newtwork protocol.
*/

void QUrl::listEntries( const QString &nameFilter, int filterSpec = QDir::DefaultFilter,
			int sortSpec = QDir::DefaultSort )
{
    clearEntries();
    if ( isLocalFile() ) {
	d->dir = QDir( d->path );
	d->dir.setNameFilter( nameFilter );
	d->dir.setMatchAllDirs( TRUE );
	if ( !d->dir.isReadable() ) {
	    QString msg = QUrl::tr( "Could not read directory\n" + d->path );
	    emit error( ErrReadDir, msg );
	    return;
	}
	
	const QFileInfoList *filist = d->dir.entryInfoList( filterSpec, sortSpec );
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
    } else if ( d->networkProtocol ) {
	emit start( ActListDirectory );
	setNameFilter( nameFilter );
	d->networkProtocol->listEntries( nameFilter, filterSpec, sortSpec );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
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
    } else if ( d->networkProtocol ) {
	d->networkProtocol->mkdir( dirname );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
}

/*!
  Tries to remove the file \a filename.
  If it has been successful the signal removed() with
  the \a filename is emitted.
  Else the error() signal is emitted.
*/

void QUrl::remove( const QString &filename )
{
    if ( isLocalFile() ) {
	QDir dir( d->path );
	if ( dir.remove( filename ) )
	    emit removed( filename );
	else {
	    QString msg = QUrl::tr( "Could not delete file\n" + filename );
	    emit error( ErrDeleteFile, msg );
	}
    } else if ( d->networkProtocol ) {
	d->networkProtocol->remove( filename );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
}

/*!
  Tries to rename the file \a oldname by \a newname.
  If it has been successful the signal itemChanged() with
  the old and new name of the file is emitted.
  Else the error() signal is emitted.
*/

void QUrl::rename( const QString &oldname, const QString &newname )
{
    if ( isLocalFile() ) {
	QDir dir( d->path );
	if ( dir.rename( oldname, newname ) )
	    emit itemChanged( oldname, newname );
    } else if ( d->networkProtocol ) {
	d->networkProtocol->rename( oldname, newname );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
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
    } else if ( d->networkProtocol ) {
	d->networkProtocol->copy( files, dest, move );
    } else
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
}

/*!
  #### todo
*/

bool QUrl::isDir()
{
    if ( isLocalFile() ) {
	if ( QFileInfo( path() ).isDir() )
	    emit urlIsDir();
	else
	    emit urlIsFile();
    } else if ( d->networkProtocol ) {
	d->networkProtocol->isDir();
    } else {
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
	return FALSE;
    }

    return TRUE;
}

/*!
  #### todo
*/

bool QUrl::isFile()
{
    if ( isLocalFile() ) {
	if ( QFileInfo( path() ).isFile() )
	    emit urlIsFile();
	else
	    emit urlIsDir();
    } else if ( d->networkProtocol ) {
	d->networkProtocol->isFile();
    } else {
	emit error( ErrUnknownProtocol, QUrl::tr( "The protocol `%1' is not supported" ).arg( d->protocol ) );
	return FALSE;
    }

    return TRUE;
}

/*!
  #### todo
*/

void QUrl::get( const QString &info )
{
    if ( d->networkProtocol )
	d->networkProtocol->get( info );
}

/*!
  #### todo
*/

void QUrl::put( const QString &data )
{
    if ( d->networkProtocol )
	d->networkProtocol->put( data );
}

/*!
  #### todo
*/

void QUrl::setNameFilter( const QString &nameFilter )
{
    d->nameFilter = nameFilter;
}

/*!
  #### todo
*/

QString QUrl::nameFilter() const
{
    return d->nameFilter;
}

/*!
  #### todo
*/

QString QUrl::toString() const
{
    if ( isLocalFile() ) {
	if ( !qNetworkProtocolRegister || ( qNetworkProtocolRegister &&
					    qNetworkProtocolRegister->count() == 0 ) )
	    return QDir::cleanDirPath( d->path );
	return d->protocol + ":" + QDir::cleanDirPath( d->path );
    } else if ( d->networkProtocol )
	return d->networkProtocol->toString();

    return QString::null;
}

/*!
  #### todo
*/

QUrl::operator QString() const
{
    return toString();
}

/*!
  #### todo
*/

bool QUrl::cdUp()
{
    d->path += "/..";
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return TRUE;
}

/*!
  #### todo
*/

void QUrl::clearEntries()
{
    d->entryMap.clear();
}

/*!
  #### todo
*/

void QUrl::addEntry( const QUrlInfo &i )
{
    d->entryMap[ i.name().stripWhiteSpace() ] = i;
}

/*!
  #### todo
*/

QUrlInfo QUrl::info( const QString &entry ) const
{
    return d->entryMap[ entry ];
}

/*!
  #### todo
*/

void QUrl::getNetworkProtocol()
{
    if ( isLocalFile() || !qNetworkProtocolRegister ) {
	d->networkProtocol = 0;
	return;
    }

    QNetworkProtocol *p = qGetNetworkProtocol( d->protocol );
    if ( !p ) {
	d->networkProtocol = 0;
	return;
    }

    d->networkProtocol = p;
    d->networkProtocol->setUrl( this );
}
