/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qnetworkprotocol.cpp#43 $
**
** Implementation of QNetworkProtocol class
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

#include "qnetworkprotocol.h"
#include "qlocalfs.h"
#include "qurloperator.h"
#include "qtimer.h"

QNetworkProtocolDict *qNetworkProtocolRegister = 0;

struct QNetworkProtocolPrivate
{
    QUrlOperator *url;
    QQueue< QNetworkOperation > operationQueue;
    QNetworkOperation *opInProgress;
    QTimer *opStartTimer, *removeTimer;
    int removeInterval;
    bool autoDelete;
    QNetworkOperation *old;
};

// NOT REVISED
/*!
  \class QNetworkProtocol qnetworkprotocol.h

  \brief This is the base class for network protocols which provides
  a common API for network protocols.

  This is a baseclass which should be used for implementations
  of network protocols which can then be used in Qt (e.g.
  in the filedialog) together with the QUrlOperator.

  The easiest way to implement a new network protocol is, to
  reimplement the operation[something]( QNetworkOperation * )
  methodes. Of course only the ones, which are supported, should
  be reimplemented. To specify which operations are supported,
  also reimplement supportedOperations() and return an int there,
  which is ore'd together using the supported operations from
  the Operation enum.

  When you implement a newtork protocol this way, be careful
  that you always emit the correct signals. Also, always emit
  the finished() signal when an operation is done (on failure or
  success!). The Qt Network Architecture relies on correctly emitted
  finished() signals.
  
  For a detailed description about the Qt Network Architecture, and
  also how to implement and use network protocols in Qt, look
  at the <a href="network.html">Qt Network Documentation</a>.
*/

/*!
  \fn void QNetworkProtocol::newChild( const QUrlInfo &i, QNetworkOperation *op )

  This signal is emitted after the list children operation was started and
  a new child (e.g. file) has been read from e.g. a list of files. \a i
  holds the information about the new child.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/


/*!
  \fn void QNetworkProtocol::finished( QNetworkOperation *op )

  This signal is emitted when an operation of some sort finished.
  This signal is emitted always, this means on success and on failure.
  \a op is the pointer to the operation object, which contains all infos
  of the operation which has been finished, including the state and so on.
  To check if the operation was successful or not, check the state and
  error code of the operation object.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::start( QNetworkOperation *op )

  Some operations (like listing children) emit this signal
  when they start.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::createdDirectory( const QUrlInfo &i, QNetworkOperation *op )

  This signal is emitted when making a directory has been succesful
  and the directory has been created. \a i holds the information
  about the new directory.
  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::removed( QNetworkOperation *op )

  This signal is emitted when removing a child (e.g. file)
  has been succesful
  and the file has been removed. \a op holds the filename
  of the removed file in the first argument, you get it
  with op->arg1().

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::itemChanged( QNetworkOperation *op )

  This signal is emitted whenever a file, which is a child of this URL,
  has been changed e.g. by successfully renaming it. \a op holds
  the original and the new filenames in the first and second arguments.
  You get them with op->arg1() and op->arg1().

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::data( const QByteArray &data, QNetworkOperation *op )

  This signal is emitted when new \a data has been received
  after e.g. calling get or put.

  \a op is the pointer to the operation object, which contains all infos
  of the operation, including the state and so on.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *op )

  When transferring data (using put or get) this signal is emitted during the progress.
  \a bytesDone tells how much bytes of \a bytesTotal are transferred, more information
  about the operation is stored in the \a op, the pointer to the network operation
  which is processed. \a bytesTotal may be -1, which means that the number of total
  bytes is not known.

  When a protocol emits this signal, QNetworkProtocol is smart enough
  to let the QUrlOperator, which is used by the network protocol, emit
  its corresponding signal.
*/

/*!
  \fn void QNetworkProtocol::connectionStateChanged( int state, const QString &data )

  This signal is emitted whenever the state of the connection of
  the network protocol is changed. \a state describes the new state,
  which is one of
  	ConHostFound,
	ConConnected,
	ConClosed
  \a data is a message text.
*/

/*!
  \enum QNetworkProtocol::State
  
  This enum contains the state which a QNetworkOperation
  can have:
  
  <ul>
  <li> \c StWaiting - The operation is in the queue of the QNetworkProtocol 
  and is waiting for being prcessed
  <li> \c StInProgress - The operation is just processed
  <li> \c StDone - The operation has been processed succesfully
  <li> \c StFailed - The operation has been processed but an error occured
  </ul>
*/

/*!
  \enum QNetworkProtocol::Operation
  
  This enum lists all possible operations which a network protocol
  can support. supportedOperations() returns an int which is or'd
  together of these values, also the type() or a QNetworkOperation
  is always one of these values.
  
  <ul>
  <li> \c OpListChildren - Listing the childrens of a URL, e.g. of a directory
  <li> \c OpMkdir - Create a directory
  <li> \c OpRemove - remove a child (e.g. file)
  <li> \c OpRename - rename a child (e.g. file )
  <li> \c OpGet - get data from a location
  <li> \c OpPut - put data to a location
  </ul>
*/

/*!
  \enum QNetworkProtocol::ConnectionState

  When the connection state of a network protocol changes, it emits
  the signal connectionStateChanged(). The first argument is one
  of following values:

  <ul>
  <li> \c ConHostFound - Host has been found
  <li> \c ConConnected - Connection to the host has been established
  <li> \c ConClosed - connection has been closed
  </ul>  
*/

/*!
  \enum QNetworkProtocol::Error
  
  When an operation failed (finished without success) the QNetworkOperation
  of the operation returns an error code, which is one of following values:
  
  <ul>
  <li>\c NoError (0) - No error occured
  <li>\c ErrValid (-1) - The URL you are operating on is not valid
  <li>\c ErrUnknownProtocol (-2) - There is no protocol implementation available for the protocol of the URL you are operating on (e.g. if the protocol is http and no http implementation has been registered)
  <li>\c ErrUnsupported (-3) - The operation is not supported by the protocol
  <li>\c ErrParse (-4) - Parse error of the URL
  <li>\c ErrLoginIncorrect (-5) - You needed to login but the username and or password are wrong
  <li>\c ErrHostNotFound (-6) - The specified host (in the URL) couldn´t be found
  <li>\c ErrListChlidren (-7) - An error occured while listing the children
  <li>\c ErrMkdir (-8) - An error occured when creating a directory
  <li>\c ErrRemove (-9) -An error occured while removing a child
  <li>\c ErrRename (-10) - An error occured while renaming a child
  <li>\c ErrGet (-11) - An error occured while getting (retrieving) data
  <li>\c ErrPut (-12) - An error occured while putting (uploading) data
  <li>\c ErrFileNotExisting (-13) - A file which is needed by the operation doesn't exist
  <li>\c ErrPermissionDenied (-14) - The permission for doing the operation has been denied
  </ul>

  When implementing custom network protocols, you should also use these
  values of error codes. If this is not possible, you can define your own ones
  by using an integer value which doesn't conflict with one of these vales.
*/

/*!
  Constructor of the network protocol baseclass. Does some initialization
  and connecting of signals and slots.
*/

QNetworkProtocol::QNetworkProtocol()
    : QObject()
{

    d = new QNetworkProtocolPrivate;
    d->url = 0;
    d->opInProgress = 0;
    d->opStartTimer = new QTimer( this );
    d->removeTimer = new QTimer( this );
    d->operationQueue.setAutoDelete( FALSE );
    d->autoDelete = FALSE;
    d->removeInterval = 10000;
    d->old = 0;
    connect( d->opStartTimer, SIGNAL( timeout() ),
	     this, SLOT( startOps() ) );
    connect( d->removeTimer, SIGNAL( timeout() ),
	     this, SLOT( removeMe() ) );

    connect( this, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
	     this, SLOT( emitData( const QByteArray &, QNetworkOperation * ) ) );
    connect( this, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( emitFinished( QNetworkOperation * ) ) );
    connect( this, SIGNAL( start( QNetworkOperation * ) ),
	     this, SLOT( emitStart( QNetworkOperation * ) ) );
    connect( this, SIGNAL( newChild( const QUrlInfo &, QNetworkOperation * ) ),
	     this, SLOT( emitNewChild( const QUrlInfo &, QNetworkOperation * ) ) );
    connect( this, SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ),
	     this, SLOT( emitCreatedDirectory( const QUrlInfo &, QNetworkOperation * ) ) );
    connect( this, SIGNAL( removed( QNetworkOperation * ) ),
	     this, SLOT( emitRemoved( QNetworkOperation * ) ) );
    connect( this, SIGNAL( itemChanged( QNetworkOperation * ) ),
	     this, SLOT( emitItemChanged( QNetworkOperation * ) ) );
    connect( this, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
	     this, SLOT( emitDataTransferProgress( int, int, QNetworkOperation * ) ) );

    connect( this, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( processNextOperation( QNetworkOperation * ) ) );

}

/*!
  Destructor.
*/

QNetworkProtocol::~QNetworkProtocol()
{
    if ( !d )
	return;
    d->removeTimer->stop();
    if ( d->opInProgress == d->operationQueue.head() )
	d->operationQueue.dequeue();
    delete d->opInProgress;
    d->operationQueue.setAutoDelete( TRUE );
    delete d->opStartTimer;
    delete d->old;
    delete d;
    d = 0;
}

/*!
  Sets the QUrlOperator, on which the protocol works.

  \sa QUrlOperator::QUrlOperator()
*/

void QNetworkProtocol::setUrl( QUrlOperator *u )
{
    d->url = u;
    if ( !d->opInProgress && !d->operationQueue.isEmpty() )
	d->opStartTimer->start( 1, TRUE );
}

/*!
  For processing operations the newtork protocol baseclass calls this
  methode quite often. This should be reimplemented by new
  network protocols. It should return TRUE, if the connection
  is ok (open), else FALSE. If the connection is not open, the protocol
  should open it. 
  
  If the connection can't be opened (e.g. because you already tried it,
  but the host couldn't be found or something like that), set the state 
  of \a op to QNetworkProtocol::StFailed and emit the finished() signal with
  this QNetworkOperation as argument.
  
  \a op is the operation which needs an open connection.
*/

bool QNetworkProtocol::checkConnection( QNetworkOperation * )
{
    return TRUE;
}

/*!
  Returns an int, which is or'd together using the enum values
  of \c QNetworkProtocol::Operation, which describes which operations 
  are supported by the network protocol. Should be reimplemented by new
  network protocols.
*/

int QNetworkProtocol::supportedOperations() const
{
    return 0;
}

/*!
  Adds the operation \a op the operation queue. The operation
  will be processed as soon as possible. This methode returns
  immediately.
*/

void QNetworkProtocol::addOperation( QNetworkOperation *op )
{
    d->operationQueue.enqueue( op );
    if ( !d->opInProgress )
	d->opStartTimer->start( 1, TRUE );
}

/*!
  Static methode to register a network protocol for Qt. E.g. if you have
  a implementation of NNTP (called Nntp), which is derived from
  QNetworkProtocol, call

  QNetworkProtocol::registerNetworkProtocol( "nntp", new QNetworkProtocolFactory<Nntp> );

  After that, this implementation is registered for nntp operations.
*/

void QNetworkProtocol::registerNetworkProtocol( const QString &protocol,
						QNetworkProtocolFactoryBase *protocolFactory )
{
    if ( !qNetworkProtocolRegister ) {
	qNetworkProtocolRegister = new QNetworkProtocolDict;
	QNetworkProtocol::registerNetworkProtocol( "file", new QNetworkProtocolFactory< QLocalFs > );
    }

    qNetworkProtocolRegister->insert( protocol, protocolFactory );
}

/*!
  Static methode to get a new instance of a network protocol. E.g. if
  you need to do some FTP operations, do

  QFtp *ftp = QNetworkProtocol::getNetworkProtocol( "ftp" );

  This returns now either NULL, if no protocol for ftp was registered,
  or a pointer to a new instance of an FTP implementation. The ownership
  of the pointer is transferred to you, so you have to delete it, if you
  don´t need it anymore.
  
  Normally you should not work directly with network protocols, so
  you will not need to call this method yourself. Rather use the
  QUrlOperator, which makes working with network protocols
  much more convenient.
  
  \sa QUrlOperator::QUrlOperator()
*/

QNetworkProtocol *QNetworkProtocol::getNetworkProtocol( const QString &protocol )
{
    if ( !qNetworkProtocolRegister ) {
	qNetworkProtocolRegister = new QNetworkProtocolDict;
	QNetworkProtocol::registerNetworkProtocol( "file", new QNetworkProtocolFactory< QLocalFs > );
    }

    if ( protocol.isNull() )
	return 0;

    QNetworkProtocolFactoryBase *factory = qNetworkProtocolRegister->find( protocol );
    if ( factory )
	return factory->createObject();

    return 0;
}

/*!
  Returns TRUE, if only a protocol for working on the local filesystem is
  registered, or FALSE if also other network protocols are registered.
*/

bool QNetworkProtocol::hasOnlyLocalFileSystem()
{
    if ( !qNetworkProtocolRegister )
	return FALSE;

    QDictIterator< QNetworkProtocolFactoryBase > it( *qNetworkProtocolRegister );
    for ( ; it.current(); ++it )
	if ( it.currentKey() != "file" )
	    return FALSE;
    return TRUE;
}

/*!
  \internal
  Starts processing network operations.
*/

void QNetworkProtocol::startOps()
{
    processNextOperation( 0 );
}

/*!
  \internal
  Processes the operation \a op. It calls the
  corresponding operation[something]( QNetworkOperation * )
  methodes.
*/

void QNetworkProtocol::processOperation( QNetworkOperation *op )
{
    if ( !op )
	return;

    switch ( op->operation() ) {
    case OpListChildren:
	operationListChildren( op );
	break;
    case OpMkdir:
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
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports listing children and
  this methode should then process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationListChildren( QNetworkOperation * )
{
}

/*!
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports making directories and
  this methode should then process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationMkDir( QNetworkOperation * )
{
}

/*!
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports removing children and
  this methode should then process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationRemove( QNetworkOperation * )
{
}

/*!
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports renaming children and
  this methode should then process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationRename( QNetworkOperation * )
{
}

/*!
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports getting data and
  process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationGet( QNetworkOperation * )
{
}

/*!
  When implemeting a new newtork protocol this methode should
  be reimplemented, if the protocol supports putting data and
  this methode should then process this QNetworkOperation.
  
  When you reimplement this method, it's very important that
  you emit the correct signals at the correct time (esp. the
  finished() signal after processing an operation). So have
  a look at the <a href="network.html">Qt Network Documentation</a>,
  there it is described in detail how to reimplement this method. Also
  you may look at the example implementation of 
  qt/extenstions/network/examples/networkprotocol/nntp.cpp.
*/

void QNetworkProtocol::operationPut( QNetworkOperation * )
{
}

/*!
  \internal
  Handles operations. Deletes the previous operation object and
  tries to process the next operation. It also checks the connection state
  and only processes the next operation, if the connection of the protocol
  is open. Else it waits until the protocol opens the connection.
*/

void QNetworkProtocol::processNextOperation( QNetworkOperation *old )
{
    d->removeTimer->stop();

    delete d->old;
    d->old = old;

    if ( d->operationQueue.isEmpty() ) {
	d->opInProgress = 0;
	if ( d->autoDelete )
	    d->removeTimer->start( d->removeInterval, TRUE );
	return;
    }

    QNetworkOperation *op = d->operationQueue.head();

    d->opInProgress = 0;

    if ( !checkConnection( op ) ) {
	if ( op->state() != QNetworkProtocol::StFailed ) {
	    d->opStartTimer->start( 1, TRUE );
	    d->opInProgress = op;
	} else {
	    d->opInProgress = op;
	    d->operationQueue.dequeue();
	    clearOperationQueue();
	    emit finished( op );
	}
	
	return;
    }

    d->opInProgress = op;
    d->operationQueue.dequeue();
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
  Returns the operation, which is just processed, or NULL
  of none is processed at the moment.
*/

QNetworkOperation *QNetworkProtocol::operationInProgress() const
{
    return d->opInProgress;
}

/*!
  Clears the opeartion queue.
*/

void QNetworkProtocol::clearOperationQueue()
{
    d->operationQueue.dequeue();
    d->opInProgress = 0;
    d->operationQueue.setAutoDelete( TRUE );
    d->operationQueue.clear();
}

/*!
  Because it's sometimes hard to care about removing network protocol
  instances, QNetworkProtocol provides an autodelete meachnism. If
  you set \a b to TRUE, this network protocol instance gets removed
  after it has been \a i milliseconds inactive (this means \a i ms after
  the last operation has been processed).
  If you set \a b to FALSE, the autodelete mechanism is switched off.

  NOTE: If you switch on autodeleting, the QNetworkProtocol also
  deletes its QUrlOperator!
*/

void QNetworkProtocol::setAutoDelete( bool b, int i )
{
    d->autoDelete = b;
    d->removeInterval = i;
}

/*!
  Returns TRUE, of autodeleting is enabled, else FALSE.

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
#if 0
	qDebug( "******************** autodelete of QNetworkProtocol %p ****************", this );
#endif
	delete url();
    }
}

/*!
  Emits the signal newChild( const QUrlInfo &, QNetworkOperation * ).
*/

void QNetworkProtocol::emitNewChild( const QUrlInfo &i, QNetworkOperation *res )
{
    if ( url() )
	url()->emitNewChild( i, res );
}

/*!
  Emits the signal finished( QNetworkOperation * ).
*/

void QNetworkProtocol::emitFinished( QNetworkOperation *res )
{
    if ( url() )
	url()->emitFinished( res );
}

/*!
  Emits the signal start( QNetworkOperation * ).
*/

void QNetworkProtocol::emitStart( QNetworkOperation *res )
{
    if ( url() )
	url()->emitStart( res );
}

/*!
  Emits the signal createdDirectory( const QUrlInfo &, QNetworkOperation *op ).
*/

void QNetworkProtocol::emitCreatedDirectory( const QUrlInfo &i, QNetworkOperation *res )
{
    if ( url() )
	url()->emitCreatedDirectory( i, res );
}

/*!
  Emits the signal removed( QNetworkOperation * ).
*/

void QNetworkProtocol::emitRemoved( QNetworkOperation *res )
{
    if ( url() )
	url()->emitRemoved( res );
}

/*!
  Emits the signal itemChanged( QNetworkOperation * ).
*/

void QNetworkProtocol::emitItemChanged( QNetworkOperation *res )
{
    if ( url() )
	url()->emitItemChanged( res );
}

/*!
  Emits the signal data( const QByteArray &, QNetworkOperation * ).
*/

void QNetworkProtocol::emitData( const QByteArray &d, QNetworkOperation *res )
{
    if ( url() )
	url()->emitData( d, res );
}

/*!
  Emits the signal dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation * ).
*/

void QNetworkProtocol::emitDataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *res )
{
    if ( url() )
	url()->emitDataTransferProgress( bytesDone, bytesTotal, res );
}



struct QNetworkOperationPrivate
{
    QNetworkProtocol::Operation operation;
    QNetworkProtocol::State state;
    QString arg1, arg2, arg3;
    QByteArray rawArg1, rawArg2, rawArg3;
    QString protocolDetail;
    int errorCode;
};

/*!
  \class QNetworkOperation qnetworkprotocol.h

  \brief This class is used to define operations for network
  protocols and return the state, arguments, etc.

  For each operation, which a network protocol should process
  such an object is created to describe the operation and the current
  state.

  \sa QNetworkProtocol::QNetworkProtocol()
*/

/*!
  Creates a network operation object. \a operation is the type
  of the operation, \a arg1, \a arg2 and  \a arg3 are the arguments
  of the operation.
  The state is initialized to StWaiting.
*/

QNetworkOperation::QNetworkOperation( QNetworkProtocol::Operation operation,
				      const QString &arg1, const QString &arg2,
				      const QString &arg3 )
{
    d = new QNetworkOperationPrivate;
    d->operation = operation;
    d->state = QNetworkProtocol::StWaiting;
    d->arg1 = arg1;
    d->arg2 = arg2;
    d->arg3 = arg3;
    d->rawArg1 = 0;
    d->rawArg2 = 0;
    d->rawArg3 = 0;
    d->protocolDetail = QString::null;
    d->errorCode = (int)QNetworkProtocol::NoError;
}

/*!
  Creates a network operation object. \a operation is the type
  of the operation, \a arg1, \a arg2 and  \a arg3 are the arguments
  of the operation in raw data.
  The state is initialized to StWaiting.
*/

QNetworkOperation::QNetworkOperation( QNetworkProtocol::Operation operation,
				      const QByteArray &arg1, const QByteArray &arg2,
				      const QByteArray &arg3 )
{
    d = new QNetworkOperationPrivate;
    d->operation = operation;
    d->state = QNetworkProtocol::StWaiting;
    d->arg1 = QString::null;
    d->arg2 = QString::null;
    d->arg3 = QString::null;
    d->rawArg1 = arg1;
    d->rawArg2 = arg2;
    d->rawArg3 = arg3;
    d->protocolDetail = QString::null;
    d->errorCode = (int)QNetworkProtocol::NoError;
}

/*!
  Destructor.
*/

QNetworkOperation::~QNetworkOperation()
{
    delete d;
    d = 0;
}

/*!
  Sets the \a state of the operation object. This should be done
  by the network protocol during processing it, and at the end
  it should be set to StDone or StFailed depending on
  success or failure.
*/

void QNetworkOperation::setState( QNetworkProtocol::State state )
{
    d->state = state;
}

/*!
  If the operation failed a \a detailed error message can be set
*/

void QNetworkOperation::setProtocolDetail( const QString &detail )
{
    d->protocolDetail = detail;
}

/*!
  If the operation failed, the protocol should set an error code
  to describe the error more detailed. Preferable one of the
  error defined in QNetworkProtocol should be used.
*/

void QNetworkOperation::setErrorCode( int ec )
{
    d->errorCode = ec;
}

/*!
  Sets the first argument of the network operation to \a arg.
*/

void QNetworkOperation::setArg1( const QString &arg )
{
    d->arg1 = arg;
}

/*!
  Sets the second argument of the network operation to \a arg.
*/

void QNetworkOperation::setArg2( const QString &arg )
{
    d->arg2 = arg;
}

/*!
  Sets the third argument of the network operation to \a arg.
*/

void QNetworkOperation::setArg3( const QString &arg )
{
    d->arg3 = arg;
}


/*!
  Sets the first raw data argument of the network operation to \a arg.
*/

void QNetworkOperation::setRawArg1( const QByteArray &arg )
{
    d->rawArg1 = arg;
}

/*!
  Sets the second raw data argument of the network operation to \a arg.
*/

void QNetworkOperation::setRawArg2( const QByteArray &arg )
{
    d->rawArg2 = arg;
}

/*!
  Sets the third raw data argument of the network operation to \a arg.
*/

void QNetworkOperation::setRawArg3( const QByteArray &arg )
{
    d->rawArg3 = arg;
}


/*!
  Returns the type of the operation.
*/

QNetworkProtocol::Operation QNetworkOperation::operation() const
{
    return d->operation;
}

/*!
  Returns the state of the operation. Using that you
  can find out if an operation is still waiting to get processed,
  if it is in process or if has been done successfully or if it failed.
*/

QNetworkProtocol::State QNetworkOperation::state() const
{
    return d->state;
}

/*!
  Returns the first argument of the operation.
*/

QString QNetworkOperation::arg1() const
{
    return d->arg1;
}

/*!
  Returns the second argument of the operation.
*/

QString QNetworkOperation::arg2() const
{
    return d->arg2;
}

/*!
  Returns the third argument of the operation.
*/

QString QNetworkOperation::arg3() const
{
    return d->arg3;
}

/*!
  Returns the first raw data argument of the operation.
*/

QByteArray QNetworkOperation::rawArg1() const
{
    return d->rawArg1;
}

/*!
  Returns the second raw data argument of the operation.
*/

QByteArray QNetworkOperation::rawArg2() const
{
    return d->rawArg2;
}

/*!
  Returns the third raw data argument of the operation.
*/

QByteArray QNetworkOperation::rawArg3() const
{
    return d->rawArg3;
}

/*!
  If the operation failed, using this method you may
  get a more detailed error message.
*/

QString QNetworkOperation::protocolDetail() const
{
    return d->protocolDetail;
}

/*!
  If an operation failed, you get the error code using
  this methode.
*/

int QNetworkOperation::errorCode() const
{
    return d->errorCode;
}
