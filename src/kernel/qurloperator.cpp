/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qurloperator.cpp#38 $
**
** Implementation of QUrlOperator class
**
** Created : 950429
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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

#include "qurloperator.h"

#ifndef QT_NO_NETWORKPROTOCOL

#include "qurlinfo.h"
#include "qnetworkprotocol.h"
#include "qmap.h"
#include "qdir.h"
#include "qptrdict.h"

//#define QURLOPERATOR_DEBUG

struct QUrlOperatorPrivate
{
    QMap<QString, QUrlInfo> entryMap;
    QNetworkProtocol *networkProtocol;
    QString nameFilter;
    QDir dir;

    // maps needed for copy/move operations
    QPtrDict<QNetworkOperation> getOpPutOpMap;
    QPtrDict<QNetworkProtocol> getOpPutProtMap;
    QPtrDict<QNetworkProtocol> getOpGetProtMap;
    QPtrDict<QNetworkOperation> getOpRemoveOpMap;
    QNetworkProtocol *currPut;
    QStringList waitingCopies;
    QString waitingCopiesDest;
    bool waitingCopiesMove;
    QList< QNetworkOperation > oldOps;
};

// NOT REVISED
/*!
  \class QUrlOperator qurloperator.h

  \brief The QUrlOperator class provides common operations on URLs
  (get() and more).

  \ingroup misc

  This class operates on hirachical structures (such as filesystems) using
  URLs. Its API allows all the common operations (listing children,
  removing children, renaming, etc.). But the class itself contains no
  functionality for that. It uses the functionality of registered network
  protocols.  Depending of the protocol of the URL, it uses a fitting
  network protocol class for the operations. In detail, each of the
  operation functions of QUrlOperator creates a QNetworkOperation object
  that describes the operation and puts it into the operation queue of the
  network protocol used.  If no fitting protocol could be found (because
  no implementation of the needed network protocol is registered), the URL
  operator emits errors. Each protocol does not support every operation,
  but error handling deals with this problem.

  A QUrlOperator can be used like this (for downloading a file):
  \code
  QUrlOperator op;
  op.copy( "ftp://ftp.trolltech.com/qt/source/qt-2.1.0.tar.gz", "file:/tmp", FALSE );
  \endcode

  Now, you will also need to connect to some signals of the QUrlOperator to be
  informed of success, errors, progress and more things.

  Of course an implementation for the FTP protocol has to be registered for this example.
  There is an implementation of the FTP protocol in the Qt Network Extension Library.

  For more information about the Qt Network Architecture see the <a href="network.html">Qt Network Documentation</a>.

  \sa QNetworkProtocol, QNetworkOperation
*/

/*!
  \fn void QUrlOperator::newChildren( const QValueList<QUrlInfo> &i, QNetworkOperation *op )

  This signal is emitted after listChildren() was called and new
  children (e.g., files) have been read from a list of files. \a i
  holds the information about the new children.\a op is the pointer to
  the operation object which contains all information about the
  operation, including the state.

  \sa QNetworkOperation, QNetworkProtocol
*/


/*!
  \fn void QUrlOperator::finished( QNetworkOperation *op )

  This signal is emitted when an operation of some sort finishes,
  whether with success or failure.  \a op is the pointer to the
  operation object, which contains all information, including the
  state, of the operation which has been finished. Check the state and
  error code of the operation object to see whether or not the
  operation was successful.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::start( QNetworkOperation *op )

  Some operations (such as listChildren()) emit this signal
  when they start processing the operation.
  \a op is the pointer to the operation object which contains all information about the operation, including the state.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::createdDirectory( const QUrlInfo &i, QNetworkOperation *op )

  This signal is emitted when mkdir() succeeds and the directory has
  been created. \a i holds the information about the new directory.
  \a op is the pointer to the operation object, which contains all
  information about the operation, including the state. Using op->arg(0)
  you also get the file name of the new directory.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::removed( QNetworkOperation *op )

  This signal is emitted when remove() has been succesful
  and the file has been removed. \a op holds the file name
  of the removed file in the first argument; you get it
  with op->arg( 0 ).

  \a op is the pointer to the operation object which contains all
  information about the operation, including the state.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::itemChanged( QNetworkOperation *op )

  This signal is emitted whenever a file which is a child of this URL
  has been changed, for example by successfully calling rename(). \a op holds
  the original and new file names in the first and second arguments.
  You get them with op->arg( 0 ) and op->arg( 1 ).

  \a op is the pointer to the operation object which contains all
  information about the operation, including the state.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::data( const QByteArray &data, QNetworkOperation *op )

  This signal is emitted when new \a data has been received after calling
  get() or put(). \op holds the name of the file whose data is retrieved
  in the first argument and the (raw) data in the second argument.  You
  get them with op->arg( 0 ) and op->rawArg( 1 ).

  \a op is the pointer to the operation object which contains all
  information about the operation, including the state.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *op )

  This signal is emitted during data transfer (using put() or
  get()). \a bytesDone tells how many bytes of \a bytesTotal are
  transferred. More information about the operation is stored in the
  \a op, the pointer to the network operation that is processed. \a
  bytesTotal may be -1, which means that the number of total bytes is
  not known.

  \sa QNetworkOperation, QNetworkProtocol
*/

/*!
  \fn void QUrlOperator::startedNextCopy( const QList<QNetworkOperation> &lst )

  This signal is emitted if copy() starts a new copy operation. \a lst
  contains all QNetworkOperations related to this copy operation.

  \sa copy()
*/

/*!
  \fn void QUrlOperator::connectionStateChanged( int state, const QString &data )

  This signal is emitted whenever the state of the connection of
  the network protocol of the URL operator changes. \a state describes the new state,
  which QNetworkProtocol::ConHostFound, QNetworkProtocol::ConConnected or
  QNetworkProtocol::ConClosed. This enum is defined in QNetworkProtocol
  \a data.
*/

/*!
  \reimp
*/

QUrlOperator::QUrlOperator()
    : QUrl()
{
    d = new QUrlOperatorPrivate;
    d->oldOps.setAutoDelete( FALSE );
    d->networkProtocol = 0;
    d->nameFilter = "*";
    d->currPut = 0;
}

/*!
  \reimp
*/

QUrlOperator::QUrlOperator( const QString &url )
    : QUrl( url )
{
    d = new QUrlOperatorPrivate;
    d->oldOps.setAutoDelete( FALSE );
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
    d->currPut = 0;
}

/*!
  Copy constructor.
*/

QUrlOperator::QUrlOperator( const QUrlOperator& url )
    : QObject(), QUrl( url )
{
    d = new QUrlOperatorPrivate;
    *d = *url.d;
    d->oldOps.setAutoDelete( FALSE );
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->nameFilter = "*";
    d->currPut = 0;
}

/*!
  \reimp
*/

QUrlOperator::QUrlOperator( const QUrlOperator& url, const QString& relUrl, bool checkSlash )
    : QUrl( url, relUrl, checkSlash )
{
    d = new QUrlOperatorPrivate;
    if ( relUrl == "." )
	*d = *url.d;
    d->oldOps.setAutoDelete( FALSE );
    d->networkProtocol = 0;
    getNetworkProtocol();
    d->currPut = 0;
}

/*!
  Destructor.
*/

QUrlOperator::~QUrlOperator()
{
    if ( !d )
	return;

    if ( d->networkProtocol )
	delete d->networkProtocol;
    while ( d->oldOps.first() ) {
	d->oldOps.first()->free();
	d->oldOps.removeFirst();
    }
    d->currPut = 0;
    delete d;
    d = 0;
}

/*!  Starts listing the children of this URL (e.g., of a
  directory). The signal start() is emitted before the first entry is
  listed and finished is emitted after the last one.  The
  newChildren() signal is emitted for each list of new entries.  If an
  error occurs, the signal finished() is emitted, so be sure to check
  the state of the network operation pointer.

  Because the operation may not be executed immediately, a pointer to the
  QNetworkOperation object created by this function is
  returned. This object contains all data about the operation and is
  used to refer to this operation later (e.g., in the signals that are emitted
  by the QUrlOperator). The return value can also be 0 if the operation object
  couldn't be created.

  The path of this QUrlOperator has to point to a directory (because
  the children of this directory will be listed), not to a
  file. Otherwise this operation might not work!
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
			  "or `%2' doesn't support listing directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!
  Tries to create a directory (child) with the name \a dirname.
  If it is successful, a newChildren()
  signal with the new child is emitted, and the
  createdDirectory() signal with
  the information about the new child is emitted, too.
  finished() (with success or failure) is also emitted
  after the operation has been processed, so check the state of the network
  operation object to see whether or not the operation was successful.

  Because the operation will not be executed immediately, a pointer to the
  QNetworkOperation object created by this function is
  returned. This object contains all data about the operation and is
  used to refer to this operation later (e.g., in the signals that are emitted
  by the QUrlOperator). The return value can also be 0 if the operation object
  couldn't be created.

  The path of this QUrlOperator has to point to a directory because
  the new directory will be created in this path, not to a
  file. Otherwise this operation might not work.
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
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!
  Tries to remove the file (child) \a filename.
  If it succeeds the signal removed() is emitted.
  finished() (with success or failure) is also emitted after
  the operation has been processed, so check the state of the network operation
  object to see whether or not the operation was successful.

  Because the operation will not be executed immediately, a pointer to the
  QNetworkOperation object created by this function is
  returned. This object contains all data about the operation and is
  used to refer to this operation later (e.g., in the signals that are emitted
  by the QUrlOperator). The return value can also be 0 if the operation object
  couldn't be created.

  The path of this QUrlOperator has to point to a directory; because
  if \a filename is relative, it will try to remove it in this
  directory, not to a file.  Otherwise this operation might not work.
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
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!
  Tries to rename the file (child) \a oldname by \a newname.
  If it succeeds, the signal itemChanged() is emitted.
  finished() (with success or failure) is also emitted after
  the operation has been processed, so check the state of the network operation
  object to see whether or not the operation was successful.

  Because the operation may not be executed immediately, a pointer to the
  QNetworkOperation object created by this function is
  returned. This object contains all data about the operation and is
  used to refer to this operation later (e.g., in the signals that are emitted
  by the QUrlOperator). The return value can also be 0 if the operation object
  couldn't be created.

  This path of this QUrlOperator has to point to a directory because \a oldname and
  \a newname are handled relative to this directory, not to a file.
  Otherwise this operation might not work!
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
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!  Copies the file \a from to \a to. If \a move is TRUE, the file is
  moved (copied and removed). \a from has to point to a file and \a to
  must point to a directory (into which \a from is copied).  The copying
  is done using the get() and put() operations. If you want to be notified
  about the progress of the operation, connect to the
  dataTransferProgress() signal. Keep in mind that the get() and put()
  operations emit this signal through the QUrlOperator. The number of
  transferred and total bytes that you receive as argument in this signal
  does not relate to the the whole copy operation; it relates first to the
  get() and then to the put() operation. Always check what type of
  operation the signal comes from - the last argument of the signal tells
  you.

  At the end, finished() (with success or failure) is emitted, so
  check the state of the network operation object to see whether or
  not the operation was successful.

  Because a move/copy operation consists of multiple operations (get(),
  put() and maybe remove()), this function doesn't return a single
  QNetworkOperation, but rather a list of them. They are in order: get(),
  put() and (if applicable) remove().

  \sa get(), put()
*/

QList<QNetworkOperation> QUrlOperator::copy( const QString &from, const QString &to, bool move )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: copy %s %s %d", from.latin1(), to.latin1(), move );
#endif

    QList<QNetworkOperation> ops;
    ops.setAutoDelete( FALSE );

    QUrlOperator *u = new QUrlOperator( *this, from );
    QString frm = *u;

    QString file = u->fileName();
    file.prepend( "/" );

    QNetworkProtocol *gProt = QNetworkProtocol::getNetworkProtocol( u->protocol() );

    if ( !gProt || !QNetworkProtocol::getNetworkProtocol( QUrl( to ).protocol() ) )
	return ops;

    u->setPath( u->dirPath() );
    gProt->setUrl( u );

    if ( gProt && ( gProt->supportedOperations() & QNetworkProtocol::OpGet ) &&
	 ( gProt->supportedOperations() & QNetworkProtocol::OpPut ) ) {
	connect( gProt, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
		 this, SLOT( copyGotData( const QByteArray &, QNetworkOperation * ) ) );
	connect( gProt, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
		 this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ) );
	connect( gProt, SIGNAL( finished( QNetworkOperation * ) ),
		 this, SLOT( continueCopy( QNetworkOperation * ) ) );
	connect( gProt, SIGNAL( finished( QNetworkOperation * ) ),
		 this, SIGNAL( finished( QNetworkOperation * ) ) );
	gProt->setAutoDelete( TRUE );
	QNetworkOperation *opGet = new QNetworkOperation( QNetworkProtocol::OpGet,
							  frm, QString::null, QString::null );
	ops.append( opGet );
	gProt->addOperation( opGet );
	QNetworkOperation *opPut = new QNetworkOperation( QNetworkProtocol::OpPut, to + file,
							  QString::null, QString::null );
	ops.append( opPut );
	QUrlOperator *u2 = new QUrlOperator( to );
	QNetworkProtocol *pProt = QNetworkProtocol::getNetworkProtocol( u2->protocol() );
	pProt->setUrl( u2 );

	connect( pProt, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
		 this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ) );
	connect( pProt, SIGNAL( finished( QNetworkOperation * ) ),
		 this, SIGNAL( finished( QNetworkOperation * ) ) );
	connect( pProt, SIGNAL( finished( QNetworkOperation * ) ),
		 this, SLOT( finishedCopy() ) );

	d->getOpPutProtMap.insert( (void*)opGet, pProt );
	d->getOpGetProtMap.insert( (void*)opGet, gProt );
	d->getOpPutOpMap.insert( (void*)opGet, opPut );

	if ( move && gProt->supportedOperations() & QNetworkProtocol::OpRemove ) {
	    QNetworkOperation *opRm = new QNetworkOperation( QNetworkProtocol::OpRemove, frm,
							     QString::null, QString::null );
	    ops.append( opRm );
	    d->getOpRemoveOpMap.insert( (void*)opGet, opRm );
	    gProt->setAutoDelete( FALSE );
	}

#ifdef QURLOPERATOR_DEBUG
	qDebug( "QUrlOperator: copy operation should start now..." );
#endif

	return ops;
    } else {
	delete gProt;
	QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet,
							frm, to, QString::null );
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support copying or moving files or directories" ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return ops;
}

/*!
  Copies \a files to the directory \a dest. If \a move is TRUE
  the files are moved, not copied. \a dest has to point to a directory.

  This function is just a more convenience version of the previous
  copy function. It calls the copy for each entry in \a files one
  after the other. You don't get a result from this function; each
  time a new copy begins, startedNextCopy() is emitted, with a list of
  QNetworkOperations that describe the new copy operation.
*/

void QUrlOperator::copy( const QStringList &files, const QString &dest,
			 bool move )
{
    d->waitingCopies = files;
    d->waitingCopiesDest = dest;
    d->waitingCopiesMove = move;

    finishedCopy();
}

/*!  Returns TRUE if the URL is a directory; otherwise it returns FALSE.
  This may not always work correctly, if the protocol of the URL is
  something other than file (local filesystem). If you pass a bool as \a
  ok argument, this is set to TRUE if the result of this function is known
  to be correct; otherwise \a ok is set to FALSE.
*/

bool QUrlOperator::isDir( bool *ok )
{
    if ( ok )
	*ok = TRUE;
    if ( isLocalFile() ) {
	if ( QFileInfo( path() ).isDir() )
	    return TRUE;
	else
	    return FALSE;
    }

    if ( d->entryMap.contains( "." ) ) {
	return d->entryMap[ "." ].isDir();
    } else {
	// #### can assume that we are a directory?
	if ( ok )
	    *ok = FALSE;
	return TRUE;
    }
}

/*!  Tells the network protocol to get data from \a location or, if
  this is QString::null, to get data from the location to which this
  URL points (see QUrl::fileName() and
  QUrl::encodedPathAndQuery()). What happens then depends on the
  network protocol.  The data() signal is emitted when data comes
  in. Because it's unlikely that all data will come in at once,
  multiple data() signals will most likely be emitted. The
  dataTransferProgress() is emitted while processing the operation.
  At the end, finished() (with success or failure) is emitted, so
  check the state of the network operation object to see whether or
  not the operation was successful.

  Now, if \a location is QString::null, the path of this QUrlOperator
  should point to a file when you use this operation. If \a location
  is not empty, it can be a relative URL (a child of the path to which
  the QUrlOperator points) or an absolute URL.

  For example, to get a web page you might do something like this:

  \code
  QUrlOperator op( "http://www.whatever.org/cgi-bin/search.pl?cmd=Hallo" );
  op.get();
  \endcode

  For most other operations, however, the path of the QUrlOperator
  must point to a directory. If you want to download a file you could
  do the following:

  \code
  QUrlOperator op( "ftp://ftp.whatever.org/pub" );
  // do some other stuff like op.listChildren() or op.mkdir( "new Dir" )
  op.get( "a_file.txt" );
  \endcode

  This will get the data of ftp://ftp.whatever.org/pub/a_file.txt.

  Never do anything like this:

  \code
  QUrlOperator op( "http://www.whatever.org/cgi-bin" );
  op.get( "search.pl?cmd=Hallo" );
  \endcode

  If \a location is not empty and relative it must not
  contain any queries or references, just the name of a child. So
  if you need to specify a query or reference, do it as shown in the first
  example or specify the full URL (such as
  http://www.whatever.org/cgi-bin/search.pl?cmd=Hallo) as \a location.

  \sa copy()
 */

const QNetworkOperation *QUrlOperator::get( const QString &location )
{
    if ( !checkValid() )
	return 0;

    QUrl u( *this );
    if ( !location.isEmpty() )
	u = QUrl( *this, location );

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpGet,
						    u,
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
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!  Tells the network protocol to put \a data in \a location. If it is
  empty (QString::null), it puts the \a data in the location to which the
  URL points. What happens depends on the network protocol. Depending on
  the network protocol, some data might come back after putting data, in
  which case the data() signal is emitted.  The dataTransferProgress() is
  emitted during processing of the operation.  At the end, finished()
  (with success or failure) is emitted, so check the state of the network
  operation object to see whether or not the operation was successful.

  Now, if \a location is QString::null, the path of this QUrlOperator should point to a file
  when you use this operation. If \a location is not empty, it can be a relative (a child of
  the path to which the QUrlOperator points) or an absolute URL.

  For putting some data to a file you can do the following:

  \code
  QUrlOperator op( "ftp://ftp.whatever.com/home/me/filename" );
  op.put( data );
  \endcode

  For most other operations, however, the path of the
  QUrlOperator must point to a directory. If you want to upload data to a file you could do the following:

  \code
  QUrlOperator op( "ftp://ftp.whatever.com/home/me" );
  // do some other stuff like op.listChildren() or op.mkdir( "new Dir" )
  op.put( data, "filename" );
  \endcode

  This will upload the data to ftp://ftp.whatever.com/home/me/filename.

  \sa copy()
 */

const QNetworkOperation *QUrlOperator::put( const QByteArray &data, const QString &location )
{
    if ( !checkValid() )
	return 0;

    QUrl u( *this );
    if ( !location.isEmpty() )
	u = QUrl( *this, location );

    QNetworkOperation *res = new QNetworkOperation( QNetworkProtocol::OpPut,
						    u, QString::null, QString::null );
    res->setRawArg( 1, data );

    if ( d->networkProtocol &&
	 d->networkProtocol->supportedOperations() & QNetworkProtocol::OpGet ) {
	d->networkProtocol->addOperation( res );
	return res;
    } else {
	QString msg = tr( "The protocol `%1' is not supported\n"
			  "or `%2' doesn't support put." ).
		      arg( protocol() ).arg( protocol() );
	res->setState( QNetworkProtocol::StFailed );
	res->setProtocolDetail( msg );
	res->setErrorCode( (int)QNetworkProtocol::ErrUnsupported );
	emit finished( res );
	deleteOperation( res );
    }

    return 0;
}

/*!
  Sets the name filter of the URL.

  \sa QDir::setNameFilter()
*/

void QUrlOperator::setNameFilter( const QString &nameFilter )
{
    d->nameFilter = nameFilter;
}

/*!
  Returns the name filter of the URL.

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

void QUrlOperator::addEntry( const QValueList<QUrlInfo> &i )
{
    QValueList<QUrlInfo>::ConstIterator it = i.begin();
    for ( ; it != i.end(); ++it )
	d->entryMap[ ( *it ).name().stripWhiteSpace() ] = *it;
}

/*!
  Returns the URL information for the child \a entry, or returns an
  empty QUrlInfo object if there is no information available
  about \a entry.
*/

QUrlInfo QUrlOperator::info( const QString &entry ) const
{
    if ( d->entryMap.contains( entry.stripWhiteSpace() ) ) {
	return d->entryMap[ entry.stripWhiteSpace() ];
     } else if ( entry == "." || entry == ".." ) {
	 // return a faked QUrlInfo
	 QUrlInfo inf;
	 inf.setName( entry );
	 inf.setDir( TRUE );
	 inf.setFile( FALSE );
	 inf.setSymLink( FALSE );
	 inf.setOwner( tr( "(unknown)" ) );
	 inf.setGroup( tr( "(unknown)" ) );
	 inf.setSize( 0 );
	 inf.setWritable( FALSE );
	 inf.setReadable( TRUE );
	 return inf;
     }

    return QUrlInfo();
}

/*!
  Finds a network protocol for the URL.
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
  Deletes the currently used network protocol.
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
    deleteNetworkProtocol();
    QUrl::operator=( url );

    QPtrDict<QNetworkOperation> getOpPutOpMap = d->getOpPutOpMap;
    QPtrDict<QNetworkProtocol> getOpPutProtMap = d->getOpPutProtMap;
    QPtrDict<QNetworkProtocol> getOpGetProtMap = d->getOpGetProtMap;
    QPtrDict<QNetworkOperation> getOpRemoveOpMap = d->getOpRemoveOpMap;

    *d = *url.d;

    d->oldOps.setAutoDelete( FALSE );
    d->getOpPutOpMap = getOpPutOpMap;
    d->getOpPutProtMap = getOpPutProtMap;
    d->getOpGetProtMap = getOpGetProtMap;
    d->getOpRemoveOpMap = getOpRemoveOpMap;

    getNetworkProtocol();
    return *this;
}

/*!
  \reimp
*/

QUrlOperator& QUrlOperator::operator=( const QString &url )
{
    deleteNetworkProtocol();
    QUrl::operator=( url );
    d->oldOps.setAutoDelete( FALSE );
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


/*!
  \internal
*/

void QUrlOperator::copyGotData( const QByteArray &data_, QNetworkOperation *op )
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: copyGotData: %d new bytes", data_.size() );
#endif
    QNetworkOperation *put = d->getOpPutOpMap[ (void*)op ];
    if ( put ) {
	QByteArray &s = put->raw( 1 );
	int size = s.size();
	s.resize( size + data_.size() );
	memcpy( s.data() + size, data_.data(), data_.size() );
    }
    emit data( data_, op );
}

/*!
  \internal
*/

void QUrlOperator::continueCopy( QNetworkOperation *op )
{
    if ( op->operation() != QNetworkProtocol::OpGet )
	return;

#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: continue copy (get finished, put will start)" );
#endif

    QNetworkOperation *put = d->getOpPutOpMap[ (void*)op ];
    QNetworkProtocol *gProt = d->getOpGetProtMap[ (void*)op ];
    QNetworkProtocol *pProt = d->getOpPutProtMap[ (void*)op ];
    QNetworkOperation *rm = d->getOpRemoveOpMap[ (void*)op ];
    d->getOpPutOpMap.take( op );
    d->getOpGetProtMap.take( op );
    d->getOpPutProtMap.take( op );
    d->getOpRemoveOpMap.take( op );
    if ( pProt )
	pProt->setAutoDelete( TRUE );
    if ( put && pProt ) {
	pProt->addOperation( put );
	d->currPut = pProt;
    }
    if ( gProt )
	gProt->setAutoDelete( TRUE );
    if ( rm && gProt )
 	gProt->addOperation( rm );
    disconnect( gProt, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
		this, SLOT( copyGotData( const QByteArray &, QNetworkOperation * ) ) );
    disconnect( gProt, SIGNAL( finished( QNetworkOperation * ) ),
		this, SLOT( continueCopy( QNetworkOperation * ) ) );
}

/*!
  \internal
*/

void QUrlOperator::finishedCopy()
{
#ifdef QURLOPERATOR_DEBUG
    qDebug( "QUrlOperator: finished copy (finished putting)" );
#endif

    d->currPut = 0;
    if ( d->waitingCopies.isEmpty() )
	return;

    QString cp = d->waitingCopies.first();
    d->waitingCopies.remove( cp );
    QList<QNetworkOperation> lst = copy( cp, d->waitingCopiesDest, d->waitingCopiesMove );
    emit startedNextCopy( lst );
}

/*!
  Stops the current network operation that was just processed and
  removes all waiting network operations of this QUrlOperator.
*/

void QUrlOperator::stop()
{
    d->getOpPutOpMap.clear();
    d->getOpRemoveOpMap.clear();
    d->getOpGetProtMap.setAutoDelete( TRUE );
    d->getOpPutProtMap.setAutoDelete( TRUE );
    QPtrDictIterator<QNetworkProtocol> it( d->getOpPutProtMap );
    for ( ; it.current(); ++it )
	it.current()->stop();
    d->getOpPutProtMap.clear();
    it = QPtrDictIterator<QNetworkProtocol>( d->getOpGetProtMap );
    for ( ; it.current(); ++it )
	it.current()->stop();
    d->getOpGetProtMap.clear();
    if ( d->currPut ) {
	d->currPut->stop();
	delete d->currPut;
	d->currPut = 0;
    }
    d->waitingCopies.clear();
    if ( d->networkProtocol )
	d->networkProtocol->stop();
    deleteNetworkProtocol();
    getNetworkProtocol();
}

/*!
  \internal
*/

void QUrlOperator::deleteOperation( QNetworkOperation *op )
{
    if ( op )
	d->oldOps.append( op );
}

#endif // QT_NO_NETWORKPROTOCOL
