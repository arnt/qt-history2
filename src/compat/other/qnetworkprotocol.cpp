/****************************************************************************
**
** Implementation of QNetworkProtocol class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qnetworkprotocol.h"

#ifndef QT_NO_NETWORKPROTOCOL

#include "qlocalfs.h"
#include "qhttp.h"
#include "qftp.h"
#include "qurloperator.h"
#include "qtimer.h"
#include "qmap.h"
#include "qhash.h"
#include "qregexp.h"

//#define QNETWORKPROTOCOL_DEBUG
#define NETWORK_OP_DELAY 1000

typedef QHash<QString, QNetworkProtocolFactoryBase *> QNetworkProtocolDict;

static QNetworkProtocolDict qNetworkProtocolRegister;
static void registerProtocols();

class QNetworkProtocolPrivate
{
public:
    QNetworkProtocolPrivate( QNetworkProtocol *p )
    {
	url = 0;
	opInProgress = 0;
	opStartTimer = new QTimer( p );
	removeTimer = new QTimer( p );
	autoDelete = FALSE;
	removeInterval = 10000;
    }

    ~QNetworkProtocolPrivate()
    {
	removeTimer->stop();
	if ( opInProgress ) {
	    if (!operationQueue.isEmpty() && opInProgress == operationQueue.first())
		operationQueue.takeAt(0);
	    opInProgress->free();
	}
	while (!operationQueue.isEmpty()) {
	    operationQueue.first()->free();
	    operationQueue.takeAt(0);
	}
	while (!oldOps.isEmpty()) {
	    oldOps.first()->free();
	    oldOps.removeFirst();
	}
	delete opStartTimer;
    }

    QUrlOperator *url;
    QList<QNetworkOperation *> operationQueue;
    QNetworkOperation *opInProgress;
    QTimer *opStartTimer, *removeTimer;
    int removeInterval;
    bool autoDelete;
    QList<QNetworkOperation *> oldOps;
};

/*!
    \class QNetworkProtocol qnetworkprotocol.h
    \brief The QNetworkProtocol class provides a common API for network protocols.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module network
    \ingroup io
    \module network
    \mainclass

    This is a base class which should be used for network protocols
    implementations that can then be used in Qt (e.g. in the file
    dialog) together with the QUrlOperator.

    The easiest way to implement a new network protocol is to
    reimplement the operation*() methods, e.g. operationGet(), etc.
    Only the supported operations should be reimplemented. To specify
    which operations are supported, also reimplement
    supportedOperations() and return an int that is OR'd together
    using the supported operations from the \l
    QNetworkProtocol::Operation enum.

    When you implement a network protocol this way, it is important to
    emit the correct signals. Also, always emit the finished() signal
    when an operation is done (on success \e and on failure). Qt
    relies on correctly emitted finished() signals.

    For a detailed description of the Qt Network Architecture and how
    to implement and use network protocols in Qt, see the \link
    network.html Qt Network Documentation\endlink.
*/

/*!
    \fn void QNetworkProtocol::newChildren( const QList<QUrlInfo> &i, QNetworkOperation *op )

    This signal is emitted after listChildren() was called and new
    children (files) have been read from the list of files. \a i holds
    the information about the new children. \a op is the pointer to
    the operation object which contains all the information about the
    operation, including the state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.

    When implementing your own network protocol and reading children,
    you usually don't read one child at once, but rather a list of
    them. That's why this signal takes a list of QUrlInfo objects. If
    you prefer to read just one child at a time you can use the
    convenience signal newChild(), which takes a single QUrlInfo
    object.
*/

/*!
    \fn void QNetworkProtocol::newChild( const QUrlInfo &i, QNetworkOperation *op )

    This signal is emitted if a new child (file) has been read.
    QNetworkProtocol automatically connects it to a slot which creates
    a list of QUrlInfo objects (with just one QUrlInfo \a i) and emits
    the newChildren() signal with this list. \a op is the pointer to
    the operation object which contains all the information about the
    operation that has finished, including the state, etc.

    This is just a convenience signal useful for implementing your own
    network protocol. In all other cases connect to the newChildren()
    signal with its list of QUrlInfo objects.
*/

/*!
    \fn void QNetworkProtocol::finished( QNetworkOperation *op )

    This signal is emitted when an operation finishes. This signal is
    always emitted, for both success and failure. \a op is the pointer
    to the operation object which contains all the information about
    the operation, including the state, etc. Check the state and error
    code of the operation object to determine whether or not the
    operation was successful.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::start( QNetworkOperation *op )

    Some operations (such as listChildren()) emit this signal when
    they start processing the operation. \a op is the pointer to the
    operation object which contains all the information about the
    operation, including the state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::createdDirectory( const QUrlInfo &i, QNetworkOperation *op )

    This signal is emitted when mkdir() has been successful and the
    directory has been created. \a i holds the information about the
    new directory. \a op is the pointer to the operation object which
    contains all the information about the operation, including the
    state, etc. Using op->arg( 0 ), you can get the file name of the
    new directory.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::removed( QNetworkOperation *op )

    This signal is emitted when remove() has been successful and the
    file has been removed. \a op holds the file name of the removed
    file in the first argument, accessible with op->arg( 0 ). \a op is
    the pointer to the operation object which contains all the
    information about the operation, including the state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::itemChanged( QNetworkOperation *op )

    This signal is emitted whenever a file which is a child of this
    URL has been changed, e.g. by successfully calling rename(). \a op
    holds the original and the new file names in the first and second
    arguments, accessible with op->arg( 0 ) and op->arg( 1 )
    respectively. \a op is the pointer to the operation object which
    contains all the information about the operation, including the
    state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::data( const QByteArray &data,
    QNetworkOperation *op )

    This signal is emitted when new \a data has been received after
    calling get() or put(). \a op holds the name of the file from
    which data is retrieved or uploaded in its first argument, and the
    (raw) data in its second argument. You can get them with
    op->arg( 0 ) and op->rawArg( 1 ). \a op is the pointer to the
    operation object, which contains all the information about the
    operation, including the state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator (which is used by the network
    protocol) emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *op )

    This signal is emitted during the transfer of data (using put() or
    get()). \a bytesDone is how many bytes of \a bytesTotal have been
    transferred. \a bytesTotal may be -1, which means that the total
    number of bytes is not known. \a op is the pointer to the
    operation object which contains all the information about the
    operation, including the state, etc.

    When a protocol emits this signal, QNetworkProtocol is smart
    enough to let the QUrlOperator, which is used by the network
    protocol, emit its corresponding signal.
*/

/*!
    \fn void QNetworkProtocol::connectionStateChanged( int state, const QString &data )

    This signal is emitted whenever the state of the connection of the
    network protocol is changed. \a state describes the new state,
    which is one of, \c ConHostFound, \c ConConnected or \c ConClosed.
    \a data is a message text.
*/

/*!
    \enum QNetworkProtocol::State

    This enum contains the state that a QNetworkOperation can have.

    \value StWaiting  The operation is in the QNetworkProtocol's queue
    waiting to be prcessed.

    \value StInProgress  The operation is being processed.

    \value StDone  The operation has been processed successfully.

    \value StFailed  The operation has been processed but an error occurred.

    \value StStopped  The operation has been processed but has been
    stopped before it finished, and is waiting to be processed.

*/

/*!
    \enum QNetworkProtocol::Operation

    This enum lists the possible operations that a network protocol
    can support. supportedOperations() returns an int of these that is
    OR'd together. Also, the type() of a QNetworkOperation is always
    one of these values.

    \value OpListChildren  List the children of a URL, e.g. of a directory.
    \value OpMkDir  Create a directory.
    \value OpRemove  Remove a child (e.g. a file).
    \value OpRename  Rename a child (e.g. a file).
    \value OpGet  Get data from a location.
    \value OpPut  Put data to a location.
*/

/*!
    \enum QNetworkProtocol::ConnectionState

    When the connection state of a network protocol changes it emits
    the signal connectionStateChanged(). The first argument is one of
    the following values:

    \value ConHostFound  Host has been found.
    \value ConConnected  Connection to the host has been established.
    \value ConClosed  Connection has been closed.
*/

/*!
    \enum QNetworkProtocol::Error

    When an operation fails (finishes unsuccessfully), the
    QNetworkOperation of the operation returns an error code which has
    one of the following values:

    \value NoError  No error occurred.

    \value ErrValid  The URL you are operating on is not valid.

    \value ErrUnknownProtocol  There is no protocol implementation
    available for the protocol of the URL you are operating on (e.g.
    if the protocol is http and no http implementation has been
    registered).

    \value ErrUnsupported  The operation is not supported by the
    protocol.

    \value ErrParse  The URL could not be parsed correctly.

    \value ErrLoginIncorrect  You needed to login but the username
    or password is wrong.

    \value ErrHostNotFound  The specified host (in the URL) couldn't
    be found.

    \value ErrListChildren  An error occurred while listing the
    children (files).

    \value ErrMkDir  An error occurred when creating a directory.

    \value ErrRemove  An error occurred when removing a child (file).

    \value ErrRename   An error occurred when renaming a child (file).

    \value ErrGet  An error occurred while getting (retrieving) data.

    \value ErrPut  An error occurred while putting (uploading) data.

    \value ErrFileNotExisting  A file which is needed by the operation
    doesn't exist.

    \value ErrPermissionDenied  Permission for doing the operation has
    been denied.

    You should also use these error codes when implementing custom
    network protocols. If this is not possible, you can define your own
    error codes by using integer values that don't conflict with any
    of these values.
*/

/*!
    Constructor of the network protocol base class. Does some
    initialization and connecting of signals and slots.

    The parameters \a parent and \a name are passed on to the
    QObject constructor.
*/

QNetworkProtocol::QNetworkProtocol( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QNetworkProtocolPrivate( this );

    connect( d->opStartTimer, SIGNAL( timeout() ),
	     this, SLOT( startOps() ) );
    connect( d->removeTimer, SIGNAL( timeout() ),
	     this, SLOT( removeMe() ) );

    if ( url() ) {
	connect( this, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
		 url(), SIGNAL( data( const QByteArray &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( finished( QNetworkOperation * ) ),
		 url(), SIGNAL( finished( QNetworkOperation * ) ) );
	connect( this, SIGNAL( start( QNetworkOperation * ) ),
		 url(), SIGNAL( start( QNetworkOperation * ) ) );
	connect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		 url(), SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		 url(), SLOT( addEntry( const QList<QUrlInfo> & ) ) );
	connect( this, SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ),
		 url(), SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( removed( QNetworkOperation * ) ),
		 url(), SIGNAL( removed( QNetworkOperation * ) ) );
	connect( this, SIGNAL( itemChanged( QNetworkOperation * ) ),
		 url(), SIGNAL( itemChanged( QNetworkOperation * ) ) );
	connect( this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
		 url(), SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ) );
	connect( this, SIGNAL( connectionStateChanged( int, const QString & ) ),
		 url(), SIGNAL( connectionStateChanged( int, const QString & ) ) );
    }

    connect( this, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( processNextOperation( QNetworkOperation * ) ) );
    connect( this, SIGNAL( newChild( const QUrlInfo &, QNetworkOperation * ) ),
	     this, SLOT( emitNewChildren( const QUrlInfo &, QNetworkOperation * ) ) );

}

/*!
    Destructor.
*/

QNetworkProtocol::~QNetworkProtocol()
{
    delete d;
}

/*!
    Sets the QUrlOperator, on which the protocol works, to \a u.

    \sa QUrlOperator
*/

void QNetworkProtocol::setUrl( QUrlOperator *u )
{
    if ( url() ) {
	disconnect( this, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
		    url(), SIGNAL( data( const QByteArray &, QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( finished( QNetworkOperation * ) ),
		    url(), SIGNAL( finished( QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( start( QNetworkOperation * ) ),
		    url(), SIGNAL( start( QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		    url(), SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		    url(), SLOT( addEntry( const QList<QUrlInfo> & ) ) );
	disconnect( this, SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ),
		    url(), SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( removed( QNetworkOperation * ) ),
		    url(), SIGNAL( removed( QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( itemChanged( QNetworkOperation * ) ),
		    url(), SIGNAL( itemChanged( QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
		    url(), SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ) );
	disconnect( this, SIGNAL( connectionStateChanged( int, const QString & ) ),
		    url(), SIGNAL( connectionStateChanged( int, const QString & ) ) );
    }


    // ### if autoDelete is TRUE, we should delete the QUrlOperator (something
    // like below; but that is not possible since it would delete this, too).
    //if ( d->autoDelete && (d->url!=u) ) {
    //    delete d->url; // destructor deletes the network protocol
    //}
    d->url = u;

    if ( url() ) {
	connect( this, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
		 url(), SIGNAL( data( const QByteArray &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( finished( QNetworkOperation * ) ),
		 url(), SIGNAL( finished( QNetworkOperation * ) ) );
	connect( this, SIGNAL( start( QNetworkOperation * ) ),
		 url(), SIGNAL( start( QNetworkOperation * ) ) );
	connect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		 url(), SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( newChildren( const QList<QUrlInfo> &, QNetworkOperation * ) ),
		 url(), SLOT( addEntry( const QList<QUrlInfo> & ) ) );
	connect( this, SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ),
		 url(), SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ) );
	connect( this, SIGNAL( removed( QNetworkOperation * ) ),
		 url(), SIGNAL( removed( QNetworkOperation * ) ) );
	connect( this, SIGNAL( itemChanged( QNetworkOperation * ) ),
		 url(), SIGNAL( itemChanged( QNetworkOperation * ) ) );
	connect( this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
		 url(), SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ) );
	connect( this, SIGNAL( connectionStateChanged( int, const QString & ) ),
		 url(), SIGNAL( connectionStateChanged( int, const QString & ) ) );
    }

    if ( !d->opInProgress && !d->operationQueue.isEmpty() )
	d->opStartTimer->start( 0, TRUE );
}

/*!
    For processing operations the network protocol base class calls
    this method quite often. This should be reimplemented by new
    network protocols. It should return TRUE if the connection is OK
    (open); otherwise it should return FALSE. If the connection is not
    open the protocol should open it.

    If the connection can't be opened (e.g. because you already tried
    but the host couldn't be found), set the state of \a op to
    QNetworkProtocol::StFailed and emit the finished() signal with
    this QNetworkOperation as argument.

    \a op is the operation that needs an open connection.
*/

bool QNetworkProtocol::checkConnection( QNetworkOperation * )
{
    return TRUE;
}

/*!
    Returns an int that is OR'd together using the enum values of
    \l{QNetworkProtocol::Operation}, which describes which operations
    are supported by the network protocol. Should be reimplemented by
    new network protocols.
*/

int QNetworkProtocol::supportedOperations() const
{
    return 0;
}

/*!
    Adds the operation \a op to the operation queue. The operation
    will be processed as soon as possible. This method returns
    immediately.
*/

void QNetworkProtocol::addOperation( QNetworkOperation *op )
{
#ifdef QNETWORKPROTOCOL_DEBUG
    qDebug( "QNetworkOperation: addOperation: %p %d", op, op->operation() );
#endif
    d->operationQueue.append(op);
    if ( !d->opInProgress )
	d->opStartTimer->start( 0, TRUE );
}

/*!
    Static method to register a network protocol for Qt. For example,
    if you have an implementation of NNTP (called Nntp) which is
    derived from QNetworkProtocol, call:
    \code
    QNetworkProtocol::registerNetworkProtocol( "nntp", new QNetworkProtocolFactory<Nntp> );
    \endcode
    after which your implementation is registered for future nntp
    operations.

    The name of the protocol is given in \a protocol and a pointer to
    the protocol factory is given in \a protocolFactory.
*/

void QNetworkProtocol::registerNetworkProtocol( const QString &protocol,
						QNetworkProtocolFactoryBase *protocolFactory )
{
    registerProtocols();

    qNetworkProtocolRegister.ensure_constructed();
    qNetworkProtocolRegister.insert( protocol, protocolFactory );
}

/*!
    Static method to get a new instance of the network protocol \a
    protocol. For example, if you need to do some FTP operations, do
    the following:
    \code
    QFtp *ftp = QNetworkProtocol::getNetworkProtocol( "ftp" );
    \endcode
    This returns a pointer to a new instance of an ftp implementation
    or null if no protocol for ftp was registered. The ownership of
    the pointer is transferred to you, so you must delete it if you
    don't need it anymore.

    Normally you should not work directly with network protocols, so
    you will not need to call this method yourself. Instead, use
    QUrlOperator, which makes working with network protocols much more
    convenient.

    \sa QUrlOperator
*/

QNetworkProtocol *QNetworkProtocol::getNetworkProtocol( const QString &protocol )
{

    if ( protocol.isNull() )
	return 0;

    registerProtocols();

    QNetworkProtocolFactoryBase *factory = *qNetworkProtocolRegister.find( protocol );
    if ( factory )
	return factory->createObject();

    return 0;
}

/*!
    Returns TRUE if the only protocol registered is for working on the
    local filesystem; returns FALSE if other network protocols are
    also registered.
*/

bool QNetworkProtocol::hasOnlyLocalFileSystem()
{
    QHash<QString, QNetworkProtocolFactoryBase *>::ConstIterator it
	= qNetworkProtocolRegister.constBegin();
    for ( ; it != qNetworkProtocolRegister.constEnd(); ++it )
	if ( it.key() != "file" )
	    return FALSE;
    return TRUE;
}

/*!
  \internal
  Starts processing network operations.
*/

void QNetworkProtocol::startOps()
{
#ifdef QNETWORKPROTOCOL_DEBUG
    qDebug( "QNetworkOperation: start processing operations" );
#endif
    processNextOperation( 0 );
}

/*!
  \internal
  Processes the operation \a op. It calls the
  corresponding operation[something]( QNetworkOperation * )
  methods.
*/

void QNetworkProtocol::processOperation( QNetworkOperation *op )
{
    if ( !op )
	return;

    switch ( op->operation() ) {
    case OpListChildren:
	operationListChildren( op );
	break;
    case OpMkDir:
	operationMkDir( op );
	break;
    case OpRemove:
	operationRemove( op );
	break;
    case OpRename:
	operationRename( op );
	break;
    case OpGet:
	operationGet( op );
	break;
    case OpPut:
	operationPut( op );
	break;
    }
}

/*!
    When implementing a new network protocol, this method should be
    reimplemented if the protocol supports listing children (files);
    this method should then process this QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which describes in
    detail how to reimplement this method. You may also want to look
    at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationListChildren( QNetworkOperation * )
{
}

/*!
    When implementing a new network protocol, this method should be
    reimplemented if the protocol supports making directories; this
    method should then process this QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which describes in
    detail how to reimplement this method. You may also want to look
    at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationMkDir( QNetworkOperation * )
{
}

/*!
    When implementing a new network protocol, this method should be
    reimplemented if the protocol supports removing children (files);
    this method should then process this QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which is describes
    in detail how to reimplement this method. You may also want to
    look at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationRemove( QNetworkOperation * )
{
}

/*!
    When implementing a new newtork protocol, this method should be
    reimplemented if the protocol supports renaming children (files);
    this method should then process this QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which describes in
    detail how to reimplement this method. You may also want to look
    at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationRename( QNetworkOperation * )
{
}

/*!
    When implementing a new network protocol, this method should be
    reimplemented if the protocol supports getting data; this method
    should then process the QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which describes in
    detail how to reimplement this method. You may also want to look
    at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationGet( QNetworkOperation * )
{
}

/*!
    When implementing a new network protocol, this method should be
    reimplemented if the protocol supports putting (uploading) data;
    this method should then process the QNetworkOperation.

    When you reimplement this method it's very important that you emit
    the correct signals at the correct time (especially the finished()
    signal after processing an operation). Take a look at the \link
    network.html Qt Network Documentation\endlink which describes in
    detail how to reimplement this method. You may also want to look
    at the example implementation in
    examples/network/networkprotocol/nntp.cpp.

    \a op is the pointer to the operation object which contains all
    the information on the operation that has finished, including the
    state, etc.
*/

void QNetworkProtocol::operationPut( QNetworkOperation * )
{
}

/*! \internal
*/

void QNetworkProtocol::operationPutChunk( QNetworkOperation * )
{
}

/*!
  \internal
  Handles operations. Deletes the previous operation object and
  tries to process the next operation. It also checks the connection state
  and only processes the next operation, if the connection of the protocol
  is open. Otherwise it waits until the protocol opens the connection.
*/

void QNetworkProtocol::processNextOperation( QNetworkOperation *old )
{
#ifdef QNETWORKPROTOCOL_DEBUG
    qDebug( "QNetworkOperation: process next operation, old: %p", old );
#endif
    d->removeTimer->stop();

    if ( old )
	d->oldOps.append( old );
    if ( d->opInProgress && d->opInProgress!=old )
	d->oldOps.append( d->opInProgress );

    if ( d->operationQueue.isEmpty() ) {
	d->opInProgress = 0;
	if ( d->autoDelete )
	    d->removeTimer->start( d->removeInterval, TRUE );
	return;
    }

    QNetworkOperation *op = d->operationQueue.isEmpty() ? 0 : d->operationQueue.first();

    d->opInProgress = op;

    if ( !checkConnection( op ) ) {
	if ( op->state() != QNetworkProtocol::StFailed ) {
	    d->opStartTimer->start( 0, TRUE );
	} else {
	    if (!d->operationQueue.isEmpty())
		d->operationQueue.takeAt(0);
	    clearOperationQueue();
	    emit finished( op );
	}

	return;
    }

    d->opInProgress = op;
    if (!d->operationQueue.isEmpty())
	d->operationQueue.takeAt(0);
    processOperation( op );
}

/*!
    Returns the QUrlOperator on which the protocol works.
*/

QUrlOperator *QNetworkProtocol::url() const
{
    return d->url;
}

/*!
    Returns the operation, which is being processed, or 0 of no
    operation is being processed at the moment.
*/

QNetworkOperation *QNetworkProtocol::operationInProgress() const
{
    return d->opInProgress;
}

/*!
    Clears the operation queue.
*/

void QNetworkProtocol::clearOperationQueue()
{
    if (!d->operationQueue.isEmpty())
	d->operationQueue.removeFirst();
    while (!d->operationQueue.isEmpty())
	delete d->operationQueue.takeFirst();
}

/*!
    Stops the current operation that is being processed and clears all
    waiting operations.
*/

void QNetworkProtocol::stop()
{
    QNetworkOperation *op = d->opInProgress;
    clearOperationQueue();
    if ( op ) {
	op->setState( StStopped );
	op->setProtocolDetail( tr( "Operation stopped by the user" ) );
	emit finished( op );
	setUrl( 0 );
	op->free();
    }
}

/*!
    Because it's sometimes hard to take care of removing network
    protocol instances, QNetworkProtocol provides an auto-delete
    mechanism. If you set \a b to TRUE, the network protocol instance
    is removed after it has been inactive for \a i milliseconds (i.e.
    \a i milliseconds after the last operation has been processed).
    If you set \a b to FALSE the auto-delete mechanism is switched
    off.

    If you switch on auto-delete, the QNetworkProtocol also deletes
    its QUrlOperator.
*/

void QNetworkProtocol::setAutoDelete( bool b, int i )
{
    d->autoDelete = b;
    d->removeInterval = i;
}

/*!
    Returns TRUE if auto-deleting is enabled; otherwise returns FALSE.

    \sa QNetworkProtocol::setAutoDelete()
*/

bool QNetworkProtocol::autoDelete() const
{
    return d->autoDelete;
}

/*!
  \internal
*/

void QNetworkProtocol::removeMe()
{
    if ( d->autoDelete ) {
#ifdef QNETWORKPROTOCOL_DEBUG
	qDebug( "QNetworkOperation:  autodelete of QNetworkProtocol %p", this );
#endif
	delete d->url; // destructor deletes the network protocol
    }
}

void QNetworkProtocol::emitNewChildren( const QUrlInfo &i, QNetworkOperation *op )
{
    QList<QUrlInfo> lst;
    lst << i;
    emit newChildren( lst, op );
}

class QNetworkOperationPrivate
{
public:
    QNetworkProtocol::Operation operation;
    QNetworkProtocol::State state;
    QMap<int, QString> args;
    QMap<int, QByteArray> rawArgs;
    QString protocolDetail;
    int errorCode;
    QTimer *deleteTimer;
};

/*!
    \class QNetworkOperation

    \brief The QNetworkOperation class provides common operations for network protocols.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module network
    \ingroup io

    An object is created to describe the operation and the current
    state for each operation that a network protocol should process.

    For a detailed description of the Qt Network Architecture and how
    to implement and use network protocols in Qt, see the \link
    network.html Qt Network Documentation\endlink.

    \sa QNetworkProtocol
*/

/*!
    Constructs a network operation object. \a operation is the type of
    the operation, and \a arg0, \a arg1 and \a arg2 are the first
    three arguments of the operation. The state is initialized to
    QNetworkProtocol::StWaiting.

    \sa QNetworkProtocol::Operation QNetworkProtocol::State
*/

QNetworkOperation::QNetworkOperation( QNetworkProtocol::Operation operation,
				      const QString &arg0, const QString &arg1,
				      const QString &arg2 )
{
    d = new QNetworkOperationPrivate;
    d->deleteTimer = new QTimer( this );
    connect( d->deleteTimer, SIGNAL( timeout() ),
	     this, SLOT( deleteMe() ) );
    d->operation = operation;
    d->state = QNetworkProtocol::StWaiting;
    d->args[ 0 ] = arg0;
    d->args[ 1 ] = arg1;
    d->args[ 2 ] = arg2;
    d->rawArgs[ 0 ] = QByteArray();
    d->rawArgs[ 1 ] = QByteArray();
    d->rawArgs[ 2 ] = QByteArray();
    d->protocolDetail = QString::null;
    d->errorCode = (int)QNetworkProtocol::NoError;
}

/*!
    Constructs a network operation object. \a operation is the type of
    the operation, and \a arg0, \a arg1 and \a arg2 are the first
    three raw data arguments of the operation. The state is
    initialized to QNetworkProtocol::StWaiting.

    \sa QNetworkProtocol::Operation QNetworkProtocol::State
*/

QNetworkOperation::QNetworkOperation( QNetworkProtocol::Operation operation,
				      const QByteArray &arg0, const QByteArray &arg1,
				      const QByteArray &arg2 )
{
    d = new QNetworkOperationPrivate;
    d->deleteTimer = new QTimer( this );
    connect( d->deleteTimer, SIGNAL( timeout() ),
	     this, SLOT( deleteMe() ) );
    d->operation = operation;
    d->state = QNetworkProtocol::StWaiting;
    d->args[ 0 ] = QString::null;
    d->args[ 1 ] = QString::null;
    d->args[ 2 ] = QString::null;
    d->rawArgs[ 0 ] = arg0;
    d->rawArgs[ 1 ] = arg1;
    d->rawArgs[ 2 ] = arg2;
    d->protocolDetail = QString::null;
    d->errorCode = (int)QNetworkProtocol::NoError;
}

/*!
    Destructor.
*/

QNetworkOperation::~QNetworkOperation()
{
    delete d;
}

/*!
    Sets the \a state of the operation object. This should be done by
    the network protocol during processing; at the end it should be
    set to QNetworkProtocol::StDone or QNetworkProtocol::StFailed,
    depending on success or failure.

    \sa QNetworkProtocol::State
*/

void QNetworkOperation::setState( QNetworkProtocol::State state )
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    d->state = state;
}

/*!
    If the operation failed, the error message can be specified as \a
    detail.
*/

void QNetworkOperation::setProtocolDetail( const QString &detail )
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    d->protocolDetail = detail;
}

/*!
    Sets the error code to \a ec.

    If the operation failed, the protocol should set an error code to
    describe the error in more detail. If possible, one of the error
    codes defined in QNetworkProtocol should be used.

    \sa setProtocolDetail() QNetworkProtocol::Error
*/

void QNetworkOperation::setErrorCode( int ec )
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    d->errorCode = ec;
}

/*!
    Sets the network operation's \a{num}-th argument to \a arg.
*/

void QNetworkOperation::setArg( int num, const QString &arg )
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    d->args[ num ] = arg;
}

/*!
    Sets the network operation's \a{num}-th raw data argument to \a arg.
*/

void QNetworkOperation::setRawArg( int num, const QByteArray &arg )
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    d->rawArgs[ num ] = arg;
}

/*!
    Returns the type of the operation.
*/

QNetworkProtocol::Operation QNetworkOperation::operation() const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->operation;
}

/*!
    Returns the state of the operation. You can determine whether an
    operation is still waiting to be processed, is being processed,
    has been processed successfully, or failed.
*/

QNetworkProtocol::State QNetworkOperation::state() const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->state;
}

/*!
    Returns the operation's \a{num}-th argument. If this argument was
    not already set, an empty string is returned.
*/

QString QNetworkOperation::arg( int num ) const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->args[ num ];
}

/*!
    Returns the operation's \a{num}-th raw data argument. If this
    argument was not already set, an empty bytearray is returned.
*/

QByteArray QNetworkOperation::rawArg( int num ) const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->rawArgs[ num ];
}

/*!
    Returns a detailed error message for the last error. This must
    have been set using setProtocolDetail().
*/

QString QNetworkOperation::protocolDetail() const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->protocolDetail;
}

/*!
    Returns the error code for the last error that occurred.
*/

int QNetworkOperation::errorCode() const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->errorCode;
}

/*!
  \internal
*/

QByteArray& QNetworkOperation::raw( int num ) const
{
    if ( d->deleteTimer->isActive() ) {
	d->deleteTimer->stop();
	d->deleteTimer->start( NETWORK_OP_DELAY );
    }
    return d->rawArgs[ num ];
}

/*!
    Sets this object to delete itself when it hasn't been used for one
    second.

    Because QNetworkOperation pointers are passed around a lot the
    QNetworkProtocol generally does not have enough knowledge to
    delete these at the correct time. If a QNetworkProtocol doesn't
    need an operation any more it will call this function instead.

    Note: you should never need to call the method yourself.
*/

void QNetworkOperation::free()
{
    d->deleteTimer->start( NETWORK_OP_DELAY );
}

/*!
  \internal
  Internal slot for auto-deletion.
*/

void QNetworkOperation::deleteMe()
{
    delete this;
}

#ifndef QT_NO_NETWORKPROTOCOL_HTTP
/**********************************************************************
 *
 * QHttp implementation of the QNetworkProtocol interface
 *
 *********************************************************************/

class QHttpProtocol : public QNetworkProtocol
{
    Q_OBJECT
public:
    QHttpProtocol( QObject* parent = 0, const char* name = 0 )
	: QNetworkProtocol(parent, name) { }

    int supportedOperations() const;

protected:
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

private slots:
    void clientReply( const QHttpResponseHeader &rep );
    void clientDone( bool );
    void clientStateChanged( int );

private:
    QHttp http;
    int bytesRead;
};

/*! \reimp
*/
int QHttpProtocol::supportedOperations() const
{
    return OpGet | OpPut;
}

/*! \reimp
*/
void QHttpProtocol::operationGet( QNetworkOperation *op )
{
    connect( &http, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    connect( &http, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    connect( &http, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "GET", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    http.setHost( u.host(), u.port() != -1 ? u.port() : 80 );
    http.request( header );
}

/*! \reimp
*/
void QHttpProtocol::operationPut( QNetworkOperation *op )
{
    connect( &http, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    connect( &http, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    connect( &http, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    bytesRead = 0;
    op->setState( StInProgress );
    QUrl u( operationInProgress()->arg( 0 ) );
    QHttpRequestHeader header( "POST", u.encodedPathAndQuery(), 1, 0 );
    header.setValue( "Host", u.host() );
    http.setHost( u.host(), u.port() != -1 ? u.port() : 80 );
    http.request( header, op->rawArg(1) );
}

void QHttpProtocol::clientReply( const QHttpResponseHeader &rep )
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
	if ( op->operation() == OpGet && http.bytesAvailable() > 0 ) {
	    QByteArray ba = http.readAll();
	    emit data( ba, op );
	    bytesRead += ba.size();
	    if ( rep.hasContentLength() ) {
		emit dataTransferProgress( bytesRead, rep.contentLength(), op );
	    }
	}
    }
}

void QHttpProtocol::clientDone( bool err )
{
    disconnect( this, SIGNAL(readyRead(const QHttpResponseHeader&)),
	    this, SLOT(clientReply(const QHttpResponseHeader&)) );
    disconnect( this, SIGNAL(done(bool)),
	    this, SLOT(clientDone(bool)) );
    disconnect( this, SIGNAL(stateChanged(int)),
	    this, SLOT(clientStateChanged(int)) );

    if ( err ) {
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setState( QNetworkProtocol::StFailed );
	    op->setProtocolDetail( http.errorString() );
	    switch ( http.error() ) {
		case QHttp::ConnectionRefused:
		    op->setErrorCode( ErrHostNotFound );
		    break;
		case QHttp::HostNotFound:
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
    } else {
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    if ( op->state() != StFailed ) {
		op->setState( QNetworkProtocol::StDone );
		op->setErrorCode( QNetworkProtocol::NoError );
	    }
	    emit finished( op );
	}
    }

}

void QHttpProtocol::clientStateChanged( int state )
{
    if ( url() ) {
	switch ( (State)state ) {
	    case QHttp::Connecting:
		emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
		break;
	    case QHttp::Sending:
		emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
		break;
	    case QHttp::Unconnected:
		emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
		break;
	    default:
		break;
	}
    } else {
	switch ( (State)state ) {
	    case QHttp::Connecting:
		emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
		break;
	    case QHttp::Sending:
		emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
		break;
	    case QHttp::Unconnected:
		emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
		break;
	    default:
		break;
	}
    }
}
#endif


#ifndef QT_NO_NETWORKPROTOCOL_FTP
/**********************************************************************
 *
 * QFtp implementation of the QNetworkProtocol interface
 *
 *********************************************************************/

class QFtpProtocol : public QNetworkProtocol
{
    Q_OBJECT

public:
    QFtpProtocol( QObject *parent = 0, const char *name = 0 )
	: QNetworkProtocol(parent, name) { npWaitForLoginDone = false; }

    int supportedOperations() const;

protected:
    void operationListChildren( QNetworkOperation *op );
    void operationMkDir( QNetworkOperation *op );
    void operationRemove( QNetworkOperation *op );
    void operationRename( QNetworkOperation *op );
    void operationGet( QNetworkOperation *op );
    void operationPut( QNetworkOperation *op );

    bool checkConnection( QNetworkOperation *op );

private slots:
    void npListInfo( const QUrlInfo & );
    void npDone( bool );
    void npStateChanged( int );
    void npDataTransferProgress( int, int );
    void npReadyRead();

private:
    QFtp ftp;
    bool npWaitForLoginDone;
};

/*!  \reimp
*/
void QFtpProtocol::operationListChildren( QNetworkOperation *op )
{
    op->setState( StInProgress );

    ftp.cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    ftp.list();
    emit start( op );
}

/*!  \reimp
*/
void QFtpProtocol::operationMkDir( QNetworkOperation *op )
{
    op->setState( StInProgress );

    ftp.mkdir( op->arg( 0 ) );
}

/*!  \reimp
*/
void QFtpProtocol::operationRemove( QNetworkOperation *op )
{
    op->setState( StInProgress );

    ftp.cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    remove( QUrl( op->arg( 0 ) ).path().ascii() );
}

/*!  \reimp
*/
void QFtpProtocol::operationRename( QNetworkOperation *op )
{
    op->setState( StInProgress );

    ftp.cd( ( url()->path().isEmpty() ? QString( "/" ) : url()->path() ) );
    ftp.rename( op->arg( 0 ), op->arg( 1 ));
}

/*!  \reimp
*/
void QFtpProtocol::operationGet( QNetworkOperation *op )
{
    op->setState( StInProgress );

    QUrl u( op->arg( 0 ) );
    ftp.get( u.path() );
}

/*!  \reimp
*/
void QFtpProtocol::operationPut( QNetworkOperation *op )
{
    op->setState( StInProgress );

    QUrl u( op->arg( 0 ) );
    ftp.put( op->rawArg(1), u.path() );
}

/*!  \reimp
*/
bool QFtpProtocol::checkConnection( QNetworkOperation *op )
{
    if ( ftp.state() == QFtp::Unconnected && !npWaitForLoginDone ) {
	connect( &ftp, SIGNAL(listInfo(const QUrlInfo &)),
		this, SLOT(npListInfo(const QUrlInfo &)) );
	connect( &ftp, SIGNAL(done(bool)),
		this, SLOT(npDone(bool)) );
	connect( &ftp, SIGNAL(stateChanged(int)),
		this, SLOT(npStateChanged(int)) );
	connect( &ftp, SIGNAL(dataTransferProgress(int,int)),
		this, SLOT(npDataTransferProgress(int,int)) );
	connect( &ftp, SIGNAL(readyRead()),
		this, SLOT(npReadyRead()) );

	npWaitForLoginDone = TRUE;
	switch ( op->operation() ) {
	    case OpGet:
	    case OpPut:
		{
		    QUrl u( op->arg( 0 ) );
		    ftp.connectToHost( u.host(), u.port() != -1 ? u.port() : 21 );
		}
		break;
	    default:
		ftp.connectToHost( url()->host(), url()->port() != -1 ? url()->port() : 21 );
		break;
	}
	QString user = url()->user().isEmpty() ? QString( "anonymous" ) : url()->user();
	QString pass = url()->password().isEmpty() ? QString( "anonymous@" ) : url()->password();
	ftp.login( user, pass );
    }

    if ( ftp.state() == QFtp::LoggedIn )
	return TRUE;
    return FALSE;
}

/*!  \reimp
*/
int QFtpProtocol::supportedOperations() const
{
    return OpListChildren | OpMkDir | OpRemove | OpRename | OpGet | OpPut;
}

void QFtpProtocol::npListInfo( const QUrlInfo & i )
{
    if ( url() ) {
	QRegExp filt( url()->nameFilter(), QString::CaseInsensitive, TRUE );
	if ( i.isDir() || filt.search( i.name() ) != -1 ) {
	    emit newChild( i, operationInProgress() );
	}
    } else {
	emit newChild( i, operationInProgress() );
    }
}

void QFtpProtocol::npDone( bool err )
{
    bool emitFinishedSignal = FALSE;
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	if ( err ) {
	    op->setProtocolDetail( ftp.errorString() );
	    op->setState( StFailed );
	    if ( ftp.error() == QFtp::HostNotFound ) {
		op->setErrorCode( (int)ErrHostNotFound );
	    } else {
		switch ( op->operation() ) {
		    case OpListChildren:
			op->setErrorCode( (int)ErrListChildren );
			break;
		    case OpMkDir:
			op->setErrorCode( (int)ErrMkDir );
			break;
		    case OpRemove:
			op->setErrorCode( (int)ErrRemove );
			break;
		    case OpRename:
			op->setErrorCode( (int)ErrRename );
			break;
		    case OpGet:
			op->setErrorCode( (int)ErrGet );
			break;
		    case OpPut:
			op->setErrorCode( (int)ErrPut );
			break;
		}
	    }
	    emitFinishedSignal = TRUE;
	} else if ( !npWaitForLoginDone ) {
	    switch ( op->operation() ) {
		case OpRemove:
		    emit removed( op );
		    break;
		case OpMkDir:
		    {
			QUrlInfo inf( op->arg( 0 ), 0, "", "", 0, QDateTime(),
				QDateTime(), TRUE, FALSE, FALSE, TRUE, TRUE, TRUE );
			emit newChild( inf, op );
			emit createdDirectory( inf, op );
		    }
		    break;
		case OpRename:
		    emit itemChanged( operationInProgress() );
		    break;
		default:
		    break;
	    }
	    op->setState( StDone );
	    emitFinishedSignal = TRUE;
	}
    }
    npWaitForLoginDone = FALSE;

    if ( ftp.state() == QFtp::Unconnected ) {
	disconnect( &ftp, SIGNAL(listInfo(const QUrlInfo &)),
		    this, SLOT(npListInfo(const QUrlInfo &)) );
	disconnect( &ftp, SIGNAL(done(bool)),
		    this, SLOT(npDone(bool)) );
	disconnect( &ftp, SIGNAL(stateChanged(int)),
		    this, SLOT(npStateChanged(int)) );
	disconnect( &ftp, SIGNAL(dataTransferProgress(int,int)),
		    this, SLOT(npDataTransferProgress(int,int)) );
	disconnect( &ftp, SIGNAL(readyRead()),
		    this, SLOT(npReadyRead()) );
    }

    // emit the finished() signal at the very end to avoid reentrance problems
    if ( emitFinishedSignal )
	emit finished( op );
}

void QFtpProtocol::npStateChanged( int state )
{
    if ( url() ) {
	if ( state == QFtp::Connecting )
	    emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
	else if ( state == QFtp::Connected )
	    emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
	else if ( state == QFtp::Unconnected )
	    emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    } else {
	if ( state == QFtp::Connecting )
	    emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
	else if ( state == QFtp::Connected )
	    emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
	else if ( state == QFtp::Unconnected )
	    emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
    }
}

void QFtpProtocol::npDataTransferProgress( int bDone, int bTotal )
{
    emit QNetworkProtocol::dataTransferProgress( bDone, bTotal, operationInProgress() );
}

void QFtpProtocol::npReadyRead()
{
    emit data( ftp.readAll(), operationInProgress() );
}
#endif

static void registerProtocols()
{
    static bool once = false;
    if (once) return;
    once = true;

    QNetworkProtocol::registerNetworkProtocol( "file", new QNetworkProtocolFactory< QLocalFs > );
#ifndef QT_NO_NETWORKPROTOCOL_FTP
    QNetworkProtocol::registerNetworkProtocol( "ftp", new QNetworkProtocolFactory< QFtpProtocol > );
#endif
#ifndef QT_NO_NETWORKPROTOCOL_HTTP
    QNetworkProtocol::registerNetworkProtocol( "http", new QNetworkProtocolFactory< QHttpProtocol > );
#endif
}

#include "qnetworkprotocol.moc"

#endif
