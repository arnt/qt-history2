/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.cpp#1 $
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

#include "qurloperator.h"
#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#include "qmap.h"
#include "qapplication.h"

struct QUrlOperatorPrivate
{
    QMap<QString, QUrlInfo> entryMap;
    QNetworkProtocol *networkProtocol;
    QString nameFilter;
    QDir dir;
};

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
  \class QUrlOperator qurloperator.h
  
  Urloperator
*/

QUrlOperator::QUrlOperator()
    : QUrl()
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    d->nameFilter = "*";
}

QUrlOperator::QUrlOperator( const QString &url )
    : QUrl( url )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
}

QUrlOperator::QUrlOperator( const QUrlOperator& url )
    : QObject(), QUrl( url )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
}

QUrlOperator::QUrlOperator( const QUrlOperator& url, const QString& relUrl_ )
    : QUrl( url, relUrl_ )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
}

QUrlOperator::~QUrlOperator()
{
    if ( d->networkProtocol )
	delete d->networkProtocol;
    delete d;
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

void QUrlOperator::listEntries()
{
    if ( !checkValid() )
	return;

    clearEntries();
    if ( isLocalFile() ) {
	d->dir = QDir( path( FALSE ) );
	d->dir.setNameFilter( d->nameFilter );
	d->dir.setMatchAllDirs( TRUE );
	if ( !d->dir.isReadable() ) {
	    QString msg = tr( "Could not read directory\n" + path( FALSE ) );
	    emit error( ErrReadDir, msg );
	    return;
	}
	
	const QFileInfoList *filist = d->dir.entryInfoList( QDir::All | QDir::Hidden );
	if ( !filist ) {
	    QString msg = tr( "Could not read directory\n" + path( FALSE ) );
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
	emit error( ErrUnknownProtocol, tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support listing directores" ).
		    arg( protocol() ).arg( protocol() ) );
}

/*!
  Tries to create a directory with the name \a dirname.
  If it has been successful an entry() signal with the
  new entry is emitted and the createdDirectory() with
  the information about the new entry is emitted too.
  Else the error() signal is emitted.
*/

void QUrlOperator::mkdir( const QString &dirname )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	d->dir = QDir( path( FALSE ) );
	if ( d->dir.mkdir( dirname ) ) {
	    QFileInfo fi( d->dir, dirname );
	    QUrlInfo inf( fi.fileName(), 0/*permissions*/, fi.owner(), fi.group(),
			  fi.size(), fi.lastModified(), fi.lastRead(), fi.isDir(), fi.isFile(),
			  fi.isSymLink(), fi.isWritable(), fi.isReadable(), fi.isExecutable() );
	    emit entry( inf );
	    emit createdDirectory( inf );
	} else {
	    QString msg = tr( "Could not create directory\n" + dirname );
	    emit error( ErrCreateDir, msg );
	}
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpMkdir ) {
	d->networkProtocol->mkdir( dirname );
    } else
	emit error( ErrUnknownProtocol, tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support making directores" ).
		    arg( protocol() ).arg( protocol() ) );
}

/*!
  Tries to remove the file \a filename.
  If it has been successful the signal removed() with
  the \a filename is emitted.
  Else the error() signal is emitted.
*/

void QUrlOperator::remove( const QString &filename )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	QDir dir( path( FALSE ) );
	if ( dir.remove( filename ) )
	    emit removed( filename );
	else {
	    QString msg = tr( "Could not delete file\n" + filename );
	    emit error( ErrDeleteFile, msg );
	}
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRemove ) {
	d->networkProtocol->remove( filename );
    } else
	emit error( ErrUnknownProtocol, tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support removing files" ).
		    arg( protocol() ).arg( protocol() ) );
}

/*!
  Tries to rename the file \a oldname by \a newname.
  If it has been successful the signal itemChanged() with
  the old and new name of the file is emitted.
  Else the error() signal is emitted.
*/

void QUrlOperator::rename( const QString &oldname, const QString &newname )
{
    if ( !checkValid() )
	return;

    if ( isLocalFile() ) {
	QDir dir( path( FALSE ) );
	if ( dir.rename( oldname, newname ) )
	    emit itemChanged( oldname, newname );
    } else if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRename ) {
	d->networkProtocol->rename( oldname, newname );
    } else
	emit error( ErrUnknownProtocol, tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support renaming files" ).
		    arg( protocol() ).arg( protocol() ) );
}

/*!
  Copies the file \a from to \a to on the local filesystem.
*/

void QUrlOperator::copy( const QString &from, const QString &to )
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

void QUrlOperator::copy( const QStringList &files, const QString &dest, bool move )
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
	emit error( ErrUnknownProtocol, tr( "The protocol `%1' is not supported\n"
						  "or `%2' doesn't support copying files" ).
		    arg( protocol() ).arg( protocol() ) );
}

/*!
  Returns TRUE if the url is a directory, else
  returns FALSE.
*/

bool QUrlOperator::isDir()
{
    if ( isLocalFile() ) {
	if ( QFileInfo( path( FALSE ) ).isDir() )
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
	if ( !path( FALSE ).isEmpty() )
	    return path( FALSE ).right( 1 ) == "/";
    }

    return FALSE;
}

/*!
 */

void QUrlOperator::get( const QCString &data )
{
    if ( d->networkProtocol &&
	d->networkProtocol->supportedOperations() & QNetworkProtocol::OpGet )
	d->networkProtocol->get( data );
}

/*!
  Sets the name filter of the URL

  \sa QDir::setNameFilter()
*/

void QUrlOperator::setNameFilter( const QString &nameFilter )
{
    d->nameFilter = nameFilter;
}

/*!
  Returns the name filter of the URL

  \sa QUrlOperator::setNameFilter() QDir::nameFilter()
*/

QString QUrlOperator::nameFilter() const
{
    return d->nameFilter;
}

/*!
  Clears the cache of file entries.
*/

void QUrlOperator::clearEntries()
{
    d->entryMap.clear();
}

/*!
  Adds an entry to the file entry cache.
*/

void QUrlOperator::addEntry( const QUrlInfo &i )
{
    d->entryMap[ i.name().stripWhiteSpace() ] = i;
}

/*!
  Returns the URL information for the file entry \a entry.
*/

QUrlInfo QUrlOperator::info( const QString &entry ) const
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

void QUrlOperator::getNetworkProtocol()
{
    if ( isLocalFile() || !qNetworkProtocolRegister ) {
	d->networkProtocol = 0;
	return;
    }

    QNetworkProtocol *p = QNetworkProtocol::getNetworkProtocol( protocol() );
    if ( !p ) {
	d->networkProtocol = 0;
	return;
    }

    d->networkProtocol = (QNetworkProtocol *)p;
    d->networkProtocol->setUrl( this );
}

/*!
 */
void QUrlOperator::deleteNetworkProtocol()
{
    if ( d->networkProtocol )
	delete d->networkProtocol;
    d->networkProtocol = 0;
}

void QUrlOperator::setPath( const QString& path )
{
    QUrl::setPath( path );
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

void QUrlOperator::reset()
{
    QUrl::reset();
    if ( d->networkProtocol )
 	delete d->networkProtocol;
    d->networkProtocol = 0;
    d->nameFilter = "*";
}

bool QUrlOperator::parse( const QString &url )
{
    bool b = QUrl::parse( url );
    if ( !b ) {
	emit error( ErrParse, tr( "Error in parsing `%1'" ).arg( url ) );
	return b;
    }
    
    if ( d->networkProtocol )
	delete d->networkProtocol;
    getNetworkProtocol();
    
    return b;
}

QUrlOperator& QUrlOperator::operator=( const QUrlOperator &url )
{
    QUrl::operator=( url );
    *d = *url.d;
    getNetworkProtocol();
    return *this;
}

bool QUrlOperator::cdUp()
{
    bool b = QUrl::cdUp();
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return b;
}

bool QUrlOperator::checkValid()
{
    if ( !isValid() ) {
	emit error( ErrValid, tr( "The entered URL is not valid!" ) );
	return FALSE;
    } else
	return TRUE;
}
