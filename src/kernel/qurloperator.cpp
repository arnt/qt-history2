/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.cpp#10 $
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
  \class QUrlOperator qurloperator.h

  \brief The QUrlOperator class provides common operations on URLs
  ("get" and more).

  This class operates on hirachical filesystems (or sort of)
  using URLs. It's API allows do all common operations on it
  (listing childeren, removing children, renaimg, etc.). But
  the class itself contains no functionality for that. It uses
  the functionality of registered network protocols. This means,
  depending of the protocol of the URL, it uses an fitting
  network protocol class for the operations. In detail, each of
  the operation methodes creates an QNetworkOperation object
  which describes the operation and puts it into the operation
  queue of the network protocol.
  If no fitting protocol could be found (is not registered),
  the url operator emits errors. Also not each protocol supports
  each operation - but the error  handling deals with this problem.

  \sa QNetworkProtocol::QNetworkProtocol()
*/

/*!
  \fn void QUrlOperator::newChild( const QUrlInfo &i, QNetworkOperation *op )

  This signal is emitted after listChildren() was called and
  a new child (e.g. file) has been read from e.g. a list of files. \a i
  holds the information about the new child.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/


/*!
  \fn void QUrlOperator::finished( QNetworkOperation *op )

  This signal is emitted when an operation of some sort finished.
  This signal is emitted always, this means on success and on failure.
  \a op is the pointer to the operation object, which contains all infos
  of the operation which has been finished, including the state and so on.
  To check if the operation was successful or not, check the state and
  error code of the operation object.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::start( QNetworkOperation *op )

  Some operations (like listChildren()) emit this signal
  when they start.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::createdDirectory( const QUrlInfo &i, QNetworkOperation *op )

  This signal is emitted when mkdir() has been succesful
  and the directory has been created. \a i holds the information
  about the new directory.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::removed( QNetworkOperation *op )

  This signal is emitted when remove() has been succesful
  and the file has been removed. \a op holds the filename
  of the removed file in the first argument, you get it
  with op->arg1().

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::itemChanged( QNetworkOperation *op )

  This signal is emitted whenever a file, which is a child of this URL,
  has been changed e.g. by successfully calling rename(). \a op holds
  the original and the new filenames in the first and second arguments.
  You get them with op->arg1() and op->arg1().

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::data( const QCString &data, QNetworkOperation *op )

  This signal is emitted when new \a data has been received
  after e.g. calling get or post.

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::copyProgress( int step, int total, QNetworkOperation *op )

  When copying a file this signal is emitted during the progress. You
  get the source and destinations usung the first two argumes of \op,
  this means with op->arg1() and op->arg2(). \a step is the progress
  (always <= \a total) or -1, if copying just started. \a total is the
  number of steps needed to copy the file.

  This signal can be used to show the progress when copying files.

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  \sa QNetworkOperation::QNetworkOperation()
*/

/*!
  \fn void QUrlOperator::emitNewChild( const QUrlInfo &, QNetworkOperation *op );

  Emits the signal newChild( const QUrlInfo &, QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitFinished( QNetworkOperation *op )

  Emits the signal finished( QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitStart( QNetworkOperation *op )

  Emits the signal start( QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitCreatedDirectory( const QUrlInfo &, QNetworkOperation *op )

  Emits the signal createdDirectory( const QUrlInfo &, QNetworkOperation *op ).
*/

/*!
  \fn void QUrlOperator::emitRemoved( QNetworkOperation *op )

  Emits the signal removed( QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitItemChanged( QNetworkOperation *op )

  Emits the signal itemChanged( QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitData( const QCString &d, QNetworkOperation *op )

  Emits the signal data( const QCString &, QNetworkOperation * ).
*/

/*!
  \fn void QUrlOperator::emitCopyProgress( int step, int total, QNetworkOperation * )

  Emits the signal copyProgress( int, int, QNetworkOperation * ).
*/

/*!
  \reimp
*/

QUrlOperator::QUrlOperator()
    : QUrl()
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    d->nameFilter = "*";
}

/*!
  \reimp
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
  Copy constructor.
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
  \reimp
*/

QUrlOperator::QUrlOperator( const QUrlOperator& url, const QString& relUrl_ )
    : QUrl( url, relUrl_ )
{
    d = new QUrlOperatorPrivate;
    d->networkProtocol = 0;
    getNetworkProtocol();
}

/*!
  Destructor.
*/

QUrlOperator::~QUrlOperator()
{
    if ( d->networkProtocol )
	delete d->networkProtocol;
    delete d;
}

/*!
  Starts listing a directory. The signal start( QNetworkOperation * )
  is emitted, before the first entry is listed, and after the last one
  finished( QNetworkOperation * ) is emitted.
  For each new entry, the newChild( QUrlInfo &, QNetworkOperation * )
  signals is emitted.
  If an error occures, also the signal finished( QNetworkOperation * )
  is emitted, so check the state of the network operation pointer!
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
  If it has been successful an newChild( QUrlInfo &, QNetworkOperation * )
  signal with the new file is emitted, and the
  createdDirectory( QUrlInfo &, QNetworkOperation * ) with
  the information about the new directory is emitted too.
  Also finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  If it has been successful the signal removed( QNetworkProtocol * ) is emitted.
  Also finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  If it has been successful the signal itemChanged( QNetworkOperation * )
  is emitted.
  Also finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  Copies the file \a from to \a to. If \a move is true,
  the file is moved (copied and removed). During the copy-process
  copyProgress( int, int, QNetworkOperation * ) is emitted.
  Also at the end finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  During the copy-process copyProgress( int, int, QNetworkOperation * )
  is emitted.
  Also at the end finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  returns FALSE. This may not always work correcrly!
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
  Tells the network protocol to get data. When data comes in,
  the data( const QCString &, QNetworkOperation * ) signal
  is emitted.
  Also at the end finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
 */

const QNetworkOperation *QUrlOperator::get()
{
    if ( !checkValid() )
	return 0;

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet,
						    QString::null,
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
  Tells the network protocol to post \a data. When data comes back,
  the data( const QCString &, QNetworkOperation * ) signal
  is emitted.
  Also at the end finished( QNetworkOperation * ) (on success or failure) is emitted,
  so check the state of the network operation object to see if the
  operation was successful or not.
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
  Clears the cache of children.
*/

void QUrlOperator::clearEntries()
{
    d->entryMap.clear();
}

/*!
  Adds an entry to the children cache.
*/

void QUrlOperator::addEntry( const QUrlInfo &i )
{
    d->entryMap[ i.name().stripWhiteSpace() ] = i;
}

/*!
  Returns the URL information for the child \a entry.
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
  Delete the currently used network protocol.
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
  \reimp
 */

QUrlOperator& QUrlOperator::operator=( const QUrlOperator &url )
{
    QUrl::operator=( url );
    *d = *url.d;
    getNetworkProtocol();
    return *this;
}

/*!
  \reimp
 */

bool QUrlOperator::cdUp()
{
    bool b = QUrl::cdUp();
    if ( d->networkProtocol )
	d->networkProtocol->setUrl( this );
    return b;
}

/*!
  \reimp
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
