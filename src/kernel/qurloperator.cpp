/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.cpp#6 $
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
  \fn void QUrlOperator::entry( const QUrlInfo &i )

  This signal is emitted after listEntries() was called and
  a new entry (file) has been read from the list of files. \a i
  holds the information about the new etry.
*/

/*!
  \fn void QUrlOperator::finished( int action )

  This signal is emitted when a data transfer of some sort finished.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrlOperator::start( int action )

  This signal is emitted when a data transfer of some sort started.
  \a action gives more information about it, this can be one of
	ActListDirectory
	ActCopyFile
	ActMoveFiles
	ActPut
*/

/*!
  \fn void QUrlOperator::createdDirectory( const QUrlInfo &i )

  This signal is emitted when mkdir() has been succesful
  and the directory has been created. \a i holds the information
  about the new directory.
*/

/*!
  \fn void QUrlOperator::removed( const QString &name )

  This signal is emitted when remove() has been succesful
  and the file has been removed. \a name is the filename
  of the removed file.
*/

/*!
  \fn void QUrlOperator::itemChanged( const QString &oldname, const QString &newname )

  This signal is emitted whenever a file, which is a child of this URL,
  has been changed e.g. by successfully calling rename(). \a oldname is
  the original name of the file and \a newname is the name which the file
  go now.
*/

/*!
  \fn void QUrlOperator::error( int ecode, const QString &msg )

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
  \fn void QUrlOperator::data( const QCString &data )

  This signal is emitted when new \a data has been received.
*/

/*!
  \fn void QUrlOperator::putSuccessful( const QString &data )

  This signal is emitted after successfully calling put(). \a data is the data
  which has been put.
*/

/*!
  \fn void QUrlOperator::copyProgress( const QString &from, const QString &to, int step, int total )

  When copying a file this signal is emitted. \a from is the file which
  is copied, \a to the destination. \a step is the progress
  (always <= \a total) or -1, if copying just started. \a total is the
  number of steps needed to copy the file.

  This signal can be used to show the progress when copying files.
*/

/*!
  \fn void QUrlOperator::emitEntry( const QUrlInfo & );

  Emits the signal entry( const QUrlInfo & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitFinished( int action )

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
  \fn void QUrlOperator::emitStart( int action )

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
  \fn void QUrlOperator::emitCreatedDirectory( const QUrlInfo & )

  Emits the signal createdDirectory( const QUrlInfo & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitRemoved( const QString & )

  Emits the signal removed( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitItemChanged( const QString &oldname, const QString &newname )

  Emits the signal itemChanged( const QString &, const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitError( int ecode, const QString &msg )

  Emits the signal error( int, const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitData( const QString &d )

  Emits the signal data( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \fn void QUrlOperator::emitPutSuccessful( const QString &d )

  Emits the signal putSuccessful( const QString & ). This method is mainly
  provided for implementations of network protocols which are
  working together with the QUrl class.
*/


/*!
  \fn void QUrlOperator::emitCopyProgress( const QString &from, const QString &to, int step, int total )

  Emits the signal copyProgress(  const QString &, const QString &, int, int).
  This method is mainly provided for implementations of network protocols which are
  working together with the QUrl class.
*/

/*!
  \class QUrlOperator qurloperator.h

  Urloperator
*/

/*!
 */

QUrlOperator::QUrlOperator()
    : QUrl()
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    d->nameFilter = "*";
}

/*!
 */

QUrlOperator::QUrlOperator( const QString &url )
    : QUrl( url )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
}

/*!
 */

QUrlOperator::QUrlOperator( const QUrlOperator& url )
    : QObject(), QUrl( url )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
}

/*!
 */

QUrlOperator::QUrlOperator( const QUrlOperator& url, const QString& relUrl_ )
    : QUrl( url, relUrl_ )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
}

/*!
 */

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

const QNetworkOperation *QUrlOperator::listChildren()
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpListChildren,
						    QString::null, QString::null, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpListChildren ) {
	d->networkProtocol->addOperation( res );
	clearEntries();
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support listing directores" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
  Tries to create a directory with the name \a dirname.
  If it has been successful an entry() signal with the
  new entry is emitted and the createdDirectory() with
  the information about the new entry is emitted too.
  Else the error() signal is emitted.
*/

const QNetworkOperation *QUrlOperator::mkdir( const QString &dirname )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpMkdir,
						    dirname, QString::null, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpMkdir ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support making directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
  Tries to remove the file \a filename.
  If it has been successful the signal removed() with
  the \a filename is emitted.
  Else the error() signal is emitted.
*/

const QNetworkOperation *QUrlOperator::remove( const QString &filename )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpRemove,
						    filename, QString::null, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRemove ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support removing files or directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
  Tries to rename the file \a oldname by \a newname.
  If it has been successful the signal itemChanged() with
  the old and new name of the file is emitted.
  Else the error() signal is emitted.
*/

const QNetworkOperation *QUrlOperator::rename( const QString &oldname, const QString &newname )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpRename,
						    oldname, newname, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpRename ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support renaming files or directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
  Copies the file \a from to \a to on the local filesystem.
*/

const QNetworkOperation *QUrlOperator::copy( const QString &from, const QString &to, bool move )
{
    if ( !checkValid() )
	return 0;

    QNetworkProtocol::Operation o = move ? QNetworkProtocol::OpMove
				    : QNetworkProtocol::OpCopy;

    QNetworkOperation *res = new QNetworkOperation( o, from, to, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & o ) {
	d->networkProtocol->addOperation( res );
	if ( move )
	    d->networkProtocol->addOperation( new QNetworkOperation( QNetworkProtocol::OpRemove,
								     from, QString::null, QString::null ) );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support copying or moving files or directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
  Copies \a files to the directory \a dest. If \a move is TRUE,
  the files are moved and not copied.
*/

QList<QNetworkOperation> QUrlOperator::copy( const QStringList &files, const QString &dest, bool move )
{
    if ( !checkValid() )
	return QList<QNetworkOperation>();
    QNetworkProtocol::Operation o = move ? QNetworkProtocol::OpMove
				    : QNetworkProtocol::OpCopy;

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & o ) {
	QStringList::ConstIterator it = files.begin();
	QList<QNetworkOperation> ops;
	for ( ; it != files.end(); ++it )
	    ops.append( copy( *it, dest, move ) );
	return ops;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support copying or moving  files or directories" ).
		      arg( protocol() ).arg( protocol() );
	QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpCopy,
							QString::null, QString::null, QString::null ); //####
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return QList<QNetworkOperation>();
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
	// #### can assume that we are a directory?
	return TRUE;
    }

    return FALSE;
}

/*!
 */

const QNetworkOperation *QUrlOperator::get( const QCString &data )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet,
						    QString::fromLatin1( data ), 
						    QString::null, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpGet ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support get" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
}

/*!
 */

const QNetworkOperation *QUrlOperator::post( const QCString &data )
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpPost,
						    QString::fromLatin1( data ), 
						    QString::null, QString::null );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpGet ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support post" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	delete res;
    }

    return 0;
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

/*!
  \reimp
*/

void QUrlOperator::setPath( const QString& path )
{
    QUrl::setPath( path );
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
}

/*!
  \reimp
*/

void QUrlOperator::reset()
{
    QUrl::reset();
    if ( d->networkProtocol )
 	delete d->networkProtocol;
    d->networkProtocol = 0;
    d->nameFilter = "*";
}

/*!
  \reimp
*/

bool QUrlOperator::parse( const QString &url )
{
    // ######
    bool b = QUrl::parse( url );
    if ( !b ) {
// 	emit error( ErrParse, tr( "Error in parsing `%1'" ).arg( url ) );
	return b;
    }

    if ( d->networkProtocol )
	delete d->networkProtocol;
    getNetworkProtocol();

    return b;
}

/*!
 */

QUrlOperator& QUrlOperator::operator=( const QUrlOperator &url )
{
    QUrl::operator=( url );
    *d = *url.d;
    getNetworkProtocol();
    return *this;
}

/*!
 */

bool QUrlOperator::cdUp()
{
    bool b = QUrl::cdUp();
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return b;
}

/*!
 */

bool QUrlOperator::checkValid()
{
    // ######
    if ( !isValid() ) {
	//emit error( ErrValid, tr( "The entered URL is not valid!" ) );
	return FALSE;
    } else
	return TRUE;
}
