/****************************************************************************
** $Id$
**
** Implementation of QFtp class.
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

#include "qftp.h"

#ifndef QT_NO_NETWORKPROTOCOL_FTP

#include "qsocket.h"
#include "qurlinfo.h"
#include "qurloperator.h"
#include "qstringlist.h"
#include "qregexp.h"
#include "qtimer.h"
#include "qfileinfo.h"
#include "qptrdict.h" // binary compatibility

//#define QFTP_DEBUG
//#define QFTP_COMMANDSOCKET_DEBUG
//#define QFTPPI_DEBUG

/*!
  \class QFtp qftp.h
    \ingroup io
  \brief The QFtp class provides an implementation of the FTP protocol.

  \module network

  This class is derived from QNetworkProtocol. QFtp is not
  normally used directly, but rather through a QUrlOperator, for
  example:

  \code
  QUrlOperator op( "ftp://ftp.trolltech.com" );
  op.listChildren(); // Asks the server to provide a directory listing
  \endcode

  This code will only work if the QFtp class is registered; to register the
  class, you must call qInitNetworkProtocols() before using a QUrlOperator
  with QFtp.

  If you really need to use QFtp directly, don't forget
  to set its QUrlOperator with setUrl().

  \sa \link network.html Qt Network Documentation \endlink QNetworkProtocol, QUrlOperator
*/

/*!
  Constructs a QFtp object.
*/

QFtp::QFtp()
    : QNetworkProtocol(), connectionReady( FALSE ),
      passiveMode( FALSE ), startGetOnFail( FALSE ),
      errorInListChildren( FALSE )
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp::QFtp" );
#endif
    init();
}



/////////////////////////////////////////////////
// new stuff
//

class QFtpPI : public QObject
{
    Q_OBJECT

public:
    enum FinishState {
	Ok,
	Error
    };

    QFtpPI( QObject *parent=0 );

    void connectToHost( const QString &host, Q_UINT16 port );

    bool sendCommands( const QStringList &cmds );
    bool sendCommand( const QString &cmd )
    { return sendCommands( QStringList( cmd ) ); }

signals:
    void connectState( int );
    void finished( int, const QString& );

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error( int );

private:
    // the states are modelled after the generalized state diagram of RFC 959,
    // page 58
    enum State {
	Begin,
	Idle,
	Waiting,
	Success,
	Failure
    };

    void processReply();
    bool startNextCmd();

    QSocket commandSocket;
    QString replyText;
    QChar replyCode[3];
    State state;
    QStringList pendingCommands;
};

QFtpPI::QFtpPI( QObject *parent ) : QObject( parent ),
    state( Begin )
{
    connect( &commandSocket, SIGNAL(hostFound()),
	    SLOT(hostFound()) );
    connect( &commandSocket, SIGNAL(connected()),
	    SLOT(connected()) );
    connect( &commandSocket, SIGNAL(connectionClosed()),
	    SLOT(connectionClosed()) );
    connect( &commandSocket, SIGNAL(delayedCloseFinished()),
	    SLOT(delayedCloseFinished()) );
    connect( &commandSocket, SIGNAL(readyRead()),
	    SLOT(readyRead()) );
    connect( &commandSocket, SIGNAL(error(int)),
	    SLOT(error(int)) );
}

void QFtpPI::connectToHost( const QString &host, Q_UINT16 port )
{
    commandSocket.connectToHost( host, port );
}

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done or an error occured, the finished() signal is emitted.

  If there are pending commands in the queue this functions returns FALSE and
  the \a cmds are not added to the queue; otherwise it returns TRUE.
*/
bool QFtpPI::sendCommands( const QStringList &cmds )
{
    if ( !pendingCommands.isEmpty() )
	return FALSE;
    pendingCommands = cmds;

    // ### What if we are not in state Idle; should we rather return FALSE.
    // Might the actual implementation result in a deadlock?
    if ( state == Idle )
	startNextCmd();
    return TRUE;
}

void QFtpPI::hostFound()
{
    emit connectState( QFtp::CsHostFound );
}

void QFtpPI::connected()
{
    state = Begin;
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [connected()]", state );
#endif
    emit connectState( QFtp::CsConnected );
    emit finished( Ok, tr( "Connected to host %1" ).arg( commandSocket.peerName() ) );
}

void QFtpPI::connectionClosed()
{
    emit connectState( QFtp::CsClosed );
}

void QFtpPI::delayedCloseFinished()
{
    emit connectState( QFtp::CsClosed );
}

void QFtpPI::error( int e )
{
    if ( e == QSocket::ErrHostNotFound ) {
	emit connectState( QFtp::CsHostNotFound );
	emit finished( Error, tr( "Host %1 not found" ).arg( commandSocket.peerName() ) );
    } else if ( e == QSocket::ErrConnectionRefused ) {
	emit connectState( QFtp::CsConnectionRefused );
	emit finished( Error, tr( "Connection refused to host %1" ).arg( commandSocket.peerName() ) );
    }
}

void QFtpPI::readyRead()
{
    while ( commandSocket.canReadLine() ) {
	// read line with respect to line continuation
	QString line = commandSocket.readLine();
	if ( replyText.isEmpty() ) {
	    if ( line.length() < 3 ) {
		// ### protocol error
		return;
	    }
	    const int lowerLimit[3] = {1,0,0};
	    const int upperLimit[3] = {5,5,9};
	    for ( int i=0; i<3; i++ ) {
		replyCode[i] = line[i];
		if ( replyCode[i].digitValue()<lowerLimit[i] || replyCode[i].digitValue()>upperLimit[i] ) {
		    // ### protocol error
		    return;
		}
	    }
	}

	QString endOfMultiLine( replyCode, 3 );
	endOfMultiLine += " ";
	QString lineCont( endOfMultiLine );
	lineCont[3] = '-';
	QString lineLeft4 = line.left(4);

	while ( lineLeft4 != endOfMultiLine ) {
	    if ( lineLeft4 == lineCont )
		replyText += line.mid( 4 ); // strip 'xyz-'
	    else
		replyText += line;
	    if ( !commandSocket.canReadLine() )
		return;
	    line = commandSocket.readLine();
	    lineLeft4 = line.left(4);
	}
	replyText += line.mid( 4 ); // strip reply code 'xyz '
	if ( replyText.endsWith("\r\n") )
	    replyText.truncate( replyText.length()-2 );

	processReply();

	replyText = "";
    }
}

void QFtpPI::processReply()
{
    // get new state
    static const State table[5] = {
	/* 1yz   2yz      3yz   4yz      5yz */
	Waiting, Success, Idle, Failure, Failure
    };
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() begin]", state );
    if ( replyText.length() < 400 )
	qDebug( "QFtpPI recv: %d %s", QString(replyCode,3).toInt(), replyText.latin1() );
    else
	qDebug( "QFtpPI recv: %d (text skipped)", QString(replyCode,3).toInt() );
#endif
    int replyType = replyCode[0].digitValue();
    switch ( state ) {
	case Begin:
	    if ( replyType == 1 ) {
		return;
	    } else if ( replyType == 2 ) {
		state = Idle;
		break;
	    }
	    // ### error handling
	    return;
	case Waiting:
	    if ( replyType<0 || replyType>5 )
		state = Failure;
	    else
		state = table[ replyType - 1 ];
	    break;
	default:
	    // ### spontaneous message
	    return;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() intermediate]", state );
#endif
    // react on new state
    switch ( state ) {
	case Begin:
	    // ### should never happen
	    break;
	case Success:
	    // ### success handling
	    state = Idle;
	    // no break!
	case Idle:
	    if ( startNextCmd() )
		state = Waiting;
	    else
		emit finished( Ok, replyText );
	    break;
	case Waiting:
	    // ### do nothing
	    break;
	case Failure:
	    emit finished( Error, replyText );
	    pendingCommands.clear();
	    state = Idle;
	    break;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply()] end", state );
#endif
}

bool QFtpPI::startNextCmd()
{
    QStringList::iterator nextCmd = pendingCommands.begin();
    if ( nextCmd == pendingCommands.end() )
	return FALSE;
#if defined(QFTPPI_DEBUG)
    qDebug( "QFtpPI send: %s", (*nextCmd).left( (*nextCmd).length()-2 ).latin1() );
#endif
    commandSocket.writeBlock( (*nextCmd).latin1(), (*nextCmd).length() );
    pendingCommands.remove( nextCmd );
    return TRUE;
}


class QFtpCommand
{
public:
    QFtpCommand( int i, QFtp::Command cmd, QStringList raw )
	: id(i), command(cmd), rawCmds(raw)
    { }

    int id;
    QFtp::Command command;
    QStringList rawCmds;
};

class QFtpPrivate
{
public:
    QFtpPrivate() : idCounter(0)
    {
	pending.setAutoDelete( TRUE );
    }

    QFtpPI pi;
    QPtrList<QFtpCommand> pending;
    int idCounter;
};

static QPtrDict<QFtpPrivate> *d_ptr = 0;
static void cleanup_d_ptr()
{
    delete d_ptr;
    d_ptr = 0;
}
static QFtpPrivate* d( const QFtp* foo )
{
    if ( !d_ptr ) {
	d_ptr = new QPtrDict<QFtpPrivate>;
	d_ptr->setAutoDelete( TRUE );
	qAddPostRoutine( cleanup_d_ptr );
    }
    QFtpPrivate* ret = d_ptr->find( (void*)foo );
    if ( ! ret ) {
	ret = new QFtpPrivate;
	d_ptr->replace( (void*) foo, ret );
    }
    return ret;
}

#if 0
static bool has_d( const QFtp* foo )
{
    return d_ptr && d_ptr->find( (void*)foo );
}
#endif

static void delete_d( const QFtp* foo )
{
    if ( d_ptr )
	d_ptr->remove( (void*) foo );
}

/*!
  Constructs a QFtp object.  The \a parent and \a name parameters are passed to
  the QObject constructor.
*/

QFtp::QFtp( QObject *parent, const char *name )
    : QNetworkProtocol(), connectionReady( FALSE ),
      passiveMode( FALSE ), startGetOnFail( FALSE ),
      errorInListChildren( FALSE )
{
    if ( parent )
	parent->insertChild( this );
    setName( name );
    init();
}

void QFtp::init()
{
    ///////////////////////////////////////////////////////////
    // ### old stuff -- cleanup when you are done with rewrite
    ///////////////////////////////////////////////////////////
    commandSocket = new QSocket( this, "command socket" );
    dataSocket = new QSocket( this, "data socket" );

    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( connectionClosed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    connect( commandSocket, SIGNAL( error( int ) ),
	     this, SLOT( error( int ) ) );
    connect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    connect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    connect( dataSocket, SIGNAL( connectionClosed() ),
	     this, SLOT( dataClosed() ) );
    connect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );
    connect( dataSocket, SIGNAL( bytesWritten( int ) ),
	     this, SLOT( dataBytesWritten( int ) ) );
    connect( dataSocket, SIGNAL( error( int ) ),
	     this, SLOT( error( int ) ) );

    ///////////////////////////////////////////////////////////
    // ### new stuff -- keep it
    ///////////////////////////////////////////////////////////
    QFtpPrivate *d = ::d( this );

    connect( &d->pi, SIGNAL(connectState(int)),
	    SIGNAL(connectState(int)) );
    connect( &d->pi, SIGNAL(finished(int,const QString&)),
	    SLOT(piFinished(int,const QString&)) );
}

/*!
  \enum QFtp::ConnectState

  This enum defines the changes of the connection state:

  \value CsHostFound if the host name lookup was successful
  \value CsConnected if the connection to the host was successful
  \value CsClosed if the connection was closed
  \value CsHostNotFound if the host name lookup failed
  \value CsConnectionRefused if the connection to the host failed.

  \sa connectState()
*/
/*!
  \enum QFtp::Command
###
*/
/*!  \fn void QFtp::connectState( int state )
  This signal is emitted when the state of the connection changes. The argument
  \a state is the new state of the connection; it is one of the enum \l
  ConnectState values.

  \sa connectToHost() ConnectState
*/
/*!  \fn void QFtp::start( int id )
  This signal is emitted ###
*/
/*!  \fn void QFtp::finishedSuccess( int id )
  This signal is emitted ###
*/
/*!  \fn void QFtp::finishedError( int id, const QString &details )
  This signal is emitted ###
*/
/*!  \fn void QFtp::doneSuccess()
  This signal is emitted ###
*/
/*!  \fn void QFtp::doneError()
  This signal is emitted ###
*/

/*!
   Connects to the FTP server \a host at the \a port. This function returns
   immediately and does not block until the connection succeeded. The
   connectState() signal is emitted when the state of the connecting
   process changes.

   Returns ###
   ### mention start() and finished...() signals

   \sa connectState()
*/
int QFtp::connectToHost( const QString &host, Q_UINT16 port )
{
    QStringList cmds;
    cmds << host;
    cmds << QString::number( (uint)port );
    return addCommand( ConnectToHost, cmds );
}

/*!
  ###
*/
int QFtp::login( const QString &user, const QString &password )
{
    QStringList cmds;
    cmds << "USER " + ( user.isNull() ? QString("anonymous") : user ) + "\r\n";
    cmds << "PASS " + ( password.isNull() ? QString("anonymous@") : password ) + "\r\n";
    return addCommand( Login, cmds );
}

/*!
  Returns the identifier of the FTP command currently executed or 0 if there is
  no command executed.

  \sa currentCommand()
*/
int QFtp::currentId() const
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return 0;
    return c->id;
}

/*!
  Returns the command type of the FTP command currently executed.

  \sa currentId()
*/
QFtp::Command QFtp::currentCommand() const
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return None;
    return c->command;
}

int QFtp::addCommand( Command cmd, const QStringList &rawCmds )
{
    QFtpPrivate *d = ::d( this );
    d->idCounter++;
    d->pending.append( new QFtpCommand(d->idCounter,cmd,rawCmds) );

    if ( d->pending.count() == 1 )
	// don't emit the start() signal before the id is returned
	QTimer::singleShot( 0, this, SLOT(startNextCommand()) );

    return d->idCounter;
}

void QFtp::startNextCommand()
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;
    emit start( c->id );
    if ( c->command == ConnectToHost ) {
	d->pi.connectToHost( c->rawCmds[0], c->rawCmds[1].toUInt() );
    } else {
	if ( !d->pi.sendCommands( c->rawCmds ) ) {
	    // ### error handling (this case should not happen)
	}
    }
}

void QFtp::piFinished( int status, const QString &text )
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;

    if ( status == QFtpPI::Ok ) {
	emit finishedSuccess( c->id );

	d->pending.removeFirst();
	if ( d->pending.isEmpty() )
	    emit doneSuccess();
	else
	    startNextCommand();
    } else {
	emit finishedError( c->id, text );

	d->pending.clear();
	emit doneError();
    }
}

//
//  end of new stuff
/////////////////////////////////////////////////






/*!
  Destructor
*/

QFtp::~QFtp()
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp::~QFtp" );
#endif

    close();
    delete commandSocket;
    delete dataSocket;
    delete_d( this );
}

/*!  \reimp
*/

void QFtp::operationListChildren( QNetworkOperation *op )
{
    op->setState( StInProgress );
    errorInListChildren = FALSE;
    passiveMode = FALSE;
#if defined(QFTP_DEBUG)
    qDebug( "QFtp: switch command socket to passive mode!" );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: PASV" );
#endif
    commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
}

/*!  \reimp
*/

void QFtp::operationMkDir( QNetworkOperation *op )
{
    op->setState( StInProgress );
    QString cmd( "MKD " + op->arg( 0 ) + "\r\n" );
    if ( QUrl::isRelativeUrl( op->arg( 0 ) ) )
	cmd = "MKD " + QUrl( *url(), op->arg( 0 ) ).path() + "\r\n";
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: %s", cmd.latin1() );
#endif
    commandSocket->writeBlock( cmd, cmd.length() );
}

/*!  \reimp
*/

void QFtp::operationRemove( QNetworkOperation *op )
{
    // put the operation in StWaiting state first until the CWD command
    // succeeds
    op->setState( StWaiting );
    QString path = url()->path().isEmpty() ? QString( "/" ) : url()->path();
    QString cmd = "CWD " + path + "\r\n";
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: %s", cmd.latin1() );
#endif
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

/*!  \reimp
*/

void QFtp::operationRename( QNetworkOperation *op )
{
    // put the operation in StWaiting state first until the CWD command
    // succeeds
    op->setState( StWaiting );
    QString path = url()->path().isEmpty() ? QString( "/" ) : url()->path();
    QString cmd = "CWD " + path + "\r\n";
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: %s", cmd.latin1() );
#endif
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

/*!  \reimp
*/

void QFtp::operationGet( QNetworkOperation *op )
{
    op->setState( StInProgress );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: TYPE I" );
#endif
    commandSocket->writeBlock( "TYPE I\r\n", 8 );
}

/*!  \reimp
*/

void QFtp::operationPut( QNetworkOperation *op )
{
    op->setState( StInProgress );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: TYPE I" );
#endif
    commandSocket->writeBlock( "TYPE I\r\n", 8 );
}

/*!  \reimp
*/

bool QFtp::checkConnection( QNetworkOperation * )
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp (" + url()->toString() + "): checkConnection" );
#endif
    if ( commandSocket->isOpen() && connectionReady && !passiveMode ) {
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: connection ok!" );
#endif
	return TRUE;
    }
    if ( commandSocket->isOpen() ) {
// #if defined(QFTP_DEBUG)
//	qDebug( "QFtp: command socket open but connection not ok!" );
// #endif
	return FALSE;
    }
    if ( commandSocket->state() == QSocket::Connecting ) {
#if defined(QFTP_DEBUG)
	qDebug( "QFtp::checkConnection(): already trying to connect" );
#endif
	return FALSE;
    }

    connectionReady = FALSE;
    commandSocket->connectToHost( url()->host(),
				  url()->port() != -1 ? url()->port() : 21 );

#if defined(QFTP_DEBUG)
	qDebug( "QFtp: try connecting!" );
#endif
    return FALSE;
}

/*!
  Closes the command and data connections to the FTP server
*/

void QFtp::close()
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp:: close and quit" );
#endif

    if ( dataSocket->isOpen() ) {
	dataSocket->close();
    }
    if ( commandSocket->isOpen() ) {
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: quit" );
#endif
	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
	commandSocket->close();
    }
}

/*!  \reimp
*/

int QFtp::supportedOperations() const
{
    return OpListChildren | OpMkDir | OpRemove | OpRename | OpGet | OpPut;
}

/*! \internal
  Parses the string, \a buffer, which is one line of a directory listing
  which came from the FTP server, and sets the values which have been
  parsed to the url info object, \a info.
*/

void QFtp::parseDir( const QString &buffer, QUrlInfo &info )
{
    QStringList lst = QStringList::split( " ", buffer );

    if ( lst.count() < 9 )
	return;

    QString tmp;

    // permissions
    tmp = lst[ 0 ];

    if ( tmp[ 0 ] == QChar( 'd' ) ) {
	info.setDir( TRUE );
	info.setFile( FALSE );
	info.setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( '-' ) ) {
	info.setDir( FALSE );
	info.setFile( TRUE );
	info.setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( 'l' ) ) {
	info.setDir( TRUE ); // #### todo
	info.setFile( FALSE );
	info.setSymLink( TRUE );
    } else {
	return;
    }

    static int user = 0;
    static int group = 1;
    static int other = 2;
    static int readable = 0;
    static int writable = 1;
    static int executable = 2;

    bool perms[ 3 ][ 3 ];
    perms[0][0] = (tmp[ 1 ] == 'r');
    perms[0][1] = (tmp[ 2 ] == 'w');
    perms[0][2] = (tmp[ 3 ] == 'x');
    perms[1][0] = (tmp[ 4 ] == 'r');
    perms[1][1] = (tmp[ 5 ] == 'w');
    perms[1][2] = (tmp[ 6 ] == 'x');
    perms[2][0] = (tmp[ 7 ] == 'r');
    perms[2][1] = (tmp[ 8 ] == 'w');
    perms[2][2] = (tmp[ 9 ] == 'x');

    // owner
    tmp = lst[ 2 ];
    info.setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info.setGroup( tmp );

    // ### not correct
    info.setWritable( ( url()->user() == info.owner() && perms[ user ][ writable ] ) ||
	perms[ other ][ writable ] );
    info.setReadable( ( url()->user() == info.owner() && perms[ user ][ readable ] ) ||
	perms[ other ][ readable ] );

    int p = 0;
    if ( perms[ user ][ readable ] )
	p |= QFileInfo::ReadUser;
    if ( perms[ user ][ writable ] )
	p |= QFileInfo::WriteUser;
    if ( perms[ user ][ executable ] )
	p |= QFileInfo::ExeUser;
    if ( perms[ group ][ readable ] )
	p |= QFileInfo::ReadGroup;
    if ( perms[ group ][ writable ] )
	p |= QFileInfo::WriteGroup;
    if ( perms[ group ][ executable ] )
	p |= QFileInfo::ExeGroup;
    if ( perms[ other ][ readable ] )
	p |= QFileInfo::ReadOther;
    if ( perms[ other ][ writable ] )
	p |= QFileInfo::WriteOther;
    if ( perms[ other ][ executable ] )
	p |= QFileInfo::ExeOther;
    info.setPermissions( p );

    // size
    tmp = lst[ 4 ];
    info.setSize( tmp.toInt() );

    // date and time
    QTime time;
    QString dateStr;
    dateStr += "Sun ";
    dateStr += lst[ 5 ];
    dateStr += ' ';
    dateStr += lst[ 6 ];
    dateStr += ' ';

    if ( lst[ 7 ].contains( ":" ) ) {
	time = QTime( lst[ 7 ].left( 2 ).toInt(), lst[ 7 ].right( 2 ).toInt() );
	dateStr += QString::number( QDate::currentDate().year() );
    } else {
	dateStr += lst[ 7 ];
    }

    QDate date = QDate::fromString( dateStr );
    info.setLastModified( QDateTime( date, time ) );

    if ( lst[ 7 ].contains( ":" ) ) {
	if( info.lastModified() > QDateTime::currentDateTime() ) {
	    QDateTime dt = info.lastModified();
	    QDate d = dt.date();
	    d.setYMD(d.year()-1, d.month(), d.day());
	    dt.setDate(d);
	    info.setLastModified(dt);
	}
    }

    // name
    if ( info.isSymLink() )
	info.setName( lst[ 8 ].stripWhiteSpace() );
    else {
	QString n;
	for ( uint i = 8; i < lst.count(); ++i )
	    n += lst[ i ] + " ";
	n = n.stripWhiteSpace();
	info.setName( n );
    }
}

/*!  \internal
*/

void QFtp::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

/*!  \internal
*/

void QFtp::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
}

/*!  \internal
*/

void QFtp::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
}

/*!  \internal
  If data has arrived on the command socket, this slot is called. The
  function looks at the data and passes it on to the function which can
  handle it
*/

void QFtp::readyRead()
{
    while ( commandSocket->canReadLine() ) {
	// read line with respect to line continuation
	static QCString s;
	QCString line = commandSocket->readLine().utf8();
	if ( s.isEmpty() )
	    s = line.left( 3 ) + " "; // add reply code for the first line
	while ( !(line[0]==s[0] && line[1]==s[1] && line[1]==s[1] && line[3]==' ') ) {
	    s += line.mid( 4 ); // strip reply codes
	    if ( !commandSocket->canReadLine() )
		return;
	    line = commandSocket->readLine().utf8();
	}
	s += line.mid( 4 );

	if ( !url() )
	    return;

	bool ok = FALSE;
	int code = s.left( 3 ).toInt( &ok );
	if ( !ok )
	    return;

#if defined(QFTP_DEBUG)
	if ( s.size() < 400 )
	    qDebug( "QFtp: readyRead; %s", s.data() );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	if ( s.size() < 400 )
	    qDebug( "QFtp R: %s", s.data() );
	else
	    qDebug( "QFtp R: More than 400 bytes received. Not printing." );
#endif

	if ( s.left( 1 ) == "1" )
	    okButTryLater( code, s );
	else if ( s.left( 1 ) == "2" )
	    okGoOn( code, s );
	else if ( s.left( 1 ) == "3" )
	    okButNeedMoreInfo( code, s );
	else if ( s.left( 1 ) == "4" )
	    errorForNow( code, s );
	else if ( s.left( 1 ) == "5" )
	    errorForgetIt( code, s );

	s = "";
    }
}

/*
  Handles responses from the server which say that
  something couldn't be done at the moment, but should be tried again later.
*/

void QFtp::okButTryLater( int, const QCString & )
{
    if ( operationInProgress() && operationInProgress()->operation() == OpPut &&
	    dataSocket && dataSocket->isOpen() ) {
	putToWrite = operationInProgress()->rawArg(1).size();
	putWritten = 0;
	dataSocket->writeBlock( operationInProgress()->rawArg(1), putToWrite );
    }
}

/*
  Handles success responses from the server. The success code is in \a
  code and the data is in \a data.
*/

void QFtp::okGoOn( int code, const QCString &data )
{
    switch ( code ) {
    case 213: { // state of a file (size and so on)
	if ( operationInProgress() ) {
	    if ( operationInProgress()->operation() == OpGet ) {
		// cut off the "213 "
		QString s( data );
		s.remove( 0, 4 );
		s = s.simplifyWhiteSpace();
		getTotalSize = s.toInt();
		operationInProgress()->setState( StInProgress );
		startGetOnFail = FALSE;
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: PASV" );
#endif
		commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	    }
	}
    } break;
    case 200: { // last command ok
	if ( operationInProgress() ) {
	    if ( operationInProgress()->operation() == OpPut ) {
		operationInProgress()->setState( StInProgress );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: PASV" );
#endif
		commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	    } else if ( operationInProgress()->operation() == OpGet ) {
		startGetOnFail = TRUE;
		getTotalSize = -1;
		getDoneSize = 0;
		QString cmd = "SIZE "+ QUrl( operationInProgress()->arg( 0 ) ).path() + "\r\n";
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: %s", cmd.latin1() );
#endif
		commandSocket->writeBlock( cmd.latin1(), cmd.length() );
	    }
	}
    } break;
    case 220: { // expect USERNAME
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: start login porcess" );
#endif
	QString user = url()->user().isEmpty() ? QString( "anonymous" ) : url()->user();
	QString cmd = "USER " + user + "\r\n";
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: write to command socket: \"%s\"", cmd.latin1() );
#endif

#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: %s", cmd.latin1() );
#endif
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
    } break;
    case 230: // succesfully logged in
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: connection is ready, successful logged int!" );
#endif
	connectionReady = TRUE;
	break;
    case 227: { // open the data connection (passive mode)
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: command socket switched to passive mode, open data connection" );
#endif
	QCString s = data;
	int i = s.find( "(" );
	int i2 = s.find( ")" );
	if ( i<0 || i2<0 ) {
#if defined(QFTP_DEBUG)
	    qDebug( "QFtp: bad 227 response -- address and port information missing" );
#endif
	    break;
	}
	s = s.mid( i + 1, i2 - i - 1 );
	if ( dataSocket->isOpen() )
	    dataSocket->close();
	QStringList lst = QStringList::split( ',', s );
	int port = ( lst[ 4 ].toInt() << 8 ) + lst[ 5 ].toInt();
	dataSocket->connectToHost( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3 ], port );
    } break;
    case 250: { // file operation succesfully
	if ( operationInProgress() && !passiveMode &&
	     operationInProgress()->operation() == OpListChildren ) { // list dir
	    if ( !errorInListChildren ) {
		operationInProgress()->setState( StInProgress );
#if defined(QFTP_DEBUG)
		qDebug( "QFtp: list children (command socket is passive!" );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: LIST" );
#endif
		commandSocket->writeBlock( "LIST\r\n", strlen( "LIST\r\n" ) );
		emit QNetworkProtocol::start( operationInProgress() );
		passiveMode = TRUE;
	    }
	} else if ( operationInProgress() &&
		    operationInProgress()->operation() == OpRename ) { // rename successfull
	    if ( operationInProgress()->state() == StWaiting ) {
		operationInProgress()->setState( StInProgress );
		QString oldname = operationInProgress()->arg( 0 );
		QString newname = operationInProgress()->arg( 1 );
		QString cmd( "RNFR " + oldname + "\r\n" );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: %s", cmd.latin1() );
#endif
		commandSocket->writeBlock( cmd, cmd.length() );
		cmd = "RNTO " + newname + "\r\n";
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: %s", cmd.latin1() );
#endif
		commandSocket->writeBlock( cmd, cmd.length() );
	    } else {
		operationInProgress()->setState( StDone );
		emit itemChanged( operationInProgress() );
		emit finished( operationInProgress() );
	    }
	} else if ( operationInProgress() &&
		    operationInProgress()->operation() == OpRemove ) { // remove or cwd successful
	    if ( operationInProgress()->state() == StWaiting ) {
		operationInProgress()->setState( StInProgress );
		QString name = QUrl( operationInProgress()->arg( 0 ) ).path();
		QString cmd( "DELE " + name + "\r\n" );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
		qDebug( "QFtp S: %s", cmd.latin1() );
#endif
		commandSocket->writeBlock( cmd, cmd.length() );
	    } else {
		operationInProgress()->setState( StDone );
		emit removed( operationInProgress() );
		emit finished( operationInProgress() );
	    }
	}
    } break;
    case 226:
	if ( !passiveMode && operationInProgress() && !errorInListChildren
		&& operationInProgress()->operation() == OpPut )
	{
	    // data socket closing
	    operationInProgress()->setState( StDone );
	    emit finished( operationInProgress() );
	}
	// else just listing directory (in passive mode) finished
	break;
    case 257: { // mkdir worked
	if ( operationInProgress() && operationInProgress()->operation() == OpMkDir ) {
	    operationInProgress()->setState( StDone );
	    // ######## todo get correct info
	    QUrlInfo inf( operationInProgress()->arg( 0 ), 0, "", "", 0, QDateTime(),
			  QDateTime(), TRUE, FALSE, FALSE, TRUE, TRUE, TRUE );
	    emit newChild( inf, operationInProgress() );
	    emit createdDirectory( inf, operationInProgress() );
	    reinitCommandSocket();
	    emit finished( operationInProgress() );
	}
    } break;
    }
}

/*
  Handles responses from the server which request more information.
  The code in \a code indicates the kind of information that is
  required.
*/

void QFtp::okButNeedMoreInfo( int code, const QCString & )
{
    switch ( code ) {
    case 331: {		// expect PASSWORD
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: write password" );
#endif
	QString pass = url()->password().isEmpty() ? QString( "anonymous@" ) : url()->password();
	QString cmd = "PASS " + pass + "\r\n";
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: write to command socket: \"%s\"", cmd.latin1() );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: %s", cmd.latin1() );
#endif
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
    } break;
    }
}

/*
  Handles error messages from the server.
*/

void QFtp::errorForNow( int, const QCString &data )
{
    QString msg( data.mid( 4 ) );
    msg = msg.simplifyWhiteSpace();
    QNetworkOperation *op = operationInProgress();
    if ( op ) {
	op->setProtocolDetail( msg );
	op->setState( StFailed );
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
	emit finished( op );
    }
    reinitCommandSocket();
}

/*
  Handles fatal error messages from the server (after this nothing more
  can be done). The error code is in \a code and the data is in \a data.
*/

void QFtp::errorForgetIt( int code, const QCString &data )
{
    if ( startGetOnFail ) {
	operationInProgress()->setState( StInProgress );
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: PASV" );
#endif
	commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	startGetOnFail = FALSE;
	return;
    }

    switch ( code ) {
    case 530: { // Login incorrect
	close();
	QString msg( tr( "Login Incorrect" ) );
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( (int)ErrLoginIncorrect );
	}
	reinitCommandSocket();
	clearOperationQueue();
	emit finished( op );
    } break;
    case 550: { // no such file or directory
	QString msg( data.mid( 4 ) );
	msg = msg.simplifyWhiteSpace();
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( (int)ErrFileNotExisting );
	}
	errorInListChildren = TRUE;
	reinitCommandSocket();
	emit finished( op );
    } break;
    case 553: { // permission denied
	QString msg( data.mid( 4 ) );
	msg = msg.simplifyWhiteSpace();
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( (int)ErrPermissionDenied );
	}
	reinitCommandSocket();
	emit finished( op );
    } break;
    default: {
	// other permanent failure (we don't know the details)
	errorForNow( code, data );
    } break;
    }
}

/*!  \internal
*/

void QFtp::dataHostFound()
{
}

/*!  \internal
  Some operations require a data connection to the server. If this connection
  could be opened, this function handles the data connection.
*/

void QFtp::dataConnected()
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp: data socket connected" );
#endif
    if ( !operationInProgress() )
	return;
    switch ( operationInProgress()->operation() ) {
    case OpListChildren: { // change dir first
	QString path = url()->path().isEmpty() ? QString( "/" ) : url()->path();
	QString cmd = "CWD " + path + "\r\n";
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: list children (data socket), to command socket write \"%s\"", cmd.latin1() );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: %s", cmd.latin1() );
#endif
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    } break;
    case OpGet: { // retrieve file
	if ( !operationInProgress() || operationInProgress()->arg( 0 ).isEmpty() ) {
	    qWarning( "no filename" );
	    break;
	}
	QString cmd = "RETR " + QUrl( operationInProgress()->arg( 0 ) ).path() + "\r\n";
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: get (data socket), to command socket write \"%s\"", cmd.latin1() );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: %s", cmd.latin1() );
#endif
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
	emit dataTransferProgress( 0, getTotalSize, operationInProgress() );
    } break;
    case OpPut: { // upload file
	if ( !operationInProgress() || operationInProgress()->arg( 0 ).isEmpty() ) {
	    qWarning( "no filename" );
	    break;
	}
	QString cmd = "STOR " + QUrl( operationInProgress()->arg( 0 ) ).path() + "\r\n";
#if defined(QFTP_DEBUG)
	qDebug( "QFtp: put (data socket), to command socket write \"%s\"", cmd.latin1() );
#endif
#if defined(QFTP_COMMANDSOCKET_DEBUG)
	qDebug( "QFtp S: %s", cmd.latin1() );
#endif
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    } break;
    case OpMkDir: {
    } break;
    case OpRemove: {
    } break;
    case OpRename: {
    } break;
    }
}

/*!  \internal
  This function is called when the data connection has been closed.
*/

void QFtp::dataClosed()
{
    if ( operationInProgress() ) {
	if ( operationInProgress()->operation() == OpPut )
	    return;

	passiveMode = FALSE;
	if ( !errorInListChildren ) {
	    operationInProgress()->setState( StDone );
	    emit finished( operationInProgress() );
	}
    }

    // switch back to ASCII mode
#if defined(QFTP_COMMANDSOCKET_DEBUG)
    qDebug( "QFtp S: TYPE A" );
#endif
    commandSocket->writeBlock( "TYPE A\r\n", 8 );

    reinitCommandSocket();
}

/*!  \internal
  This function is called when new data arrived on the data socket.
*/

void QFtp::dataReadyRead()
{
#if defined(QFTP_DEBUG)
    qDebug( "QFtp: read on data socket" );
#endif
    if ( !operationInProgress() )
	return;

    switch ( operationInProgress()->operation() ) {
    case OpListChildren: { // parse directory entry
	if ( !dataSocket->canReadLine() )
	    break;
	while ( dataSocket->canReadLine() ) {
	    QString ss = dataSocket->readLine();
	    ss = ss.stripWhiteSpace();
	    QUrlInfo inf;
	    parseDir( ss, inf );
	    if ( !inf.name().isEmpty() ) {
		if ( url() ) {
		    QRegExp filt( url()->nameFilter(), FALSE, TRUE );
		    if ( inf.isDir() || filt.search( inf.name() ) != -1 ) {
			emit newChild( inf, operationInProgress() );
		    }
		}
	    }
	}
    } break;
    case OpGet: {
	QByteArray s;
	int bytesAvailable = dataSocket->bytesAvailable();
	s.resize( bytesAvailable );
	int bytesRead = dataSocket->readBlock( s.data(), bytesAvailable );
	if ( bytesRead <= 0 )
	    break; // error
	if ( bytesRead != bytesAvailable )
	    s.resize( bytesRead );
	emit data( s, operationInProgress() );
	getDoneSize += bytesRead;
	emit dataTransferProgress( getDoneSize, getTotalSize, operationInProgress() );
	// qDebug( "%s", s.data() );
    } break;
    case OpMkDir: {
    } break;
    case OpRemove: {
    } break;
    case OpRename: {
    } break;
    case OpPut: {
    } break;
    }
}

/*!  \internal
  This function is called, when \a nbytes have been successfully written
  to the data socket.
*/

void QFtp::dataBytesWritten( int nbytes )
{
    putWritten += nbytes;
    emit dataTransferProgress( putWritten, putToWrite, operationInProgress() );
    if ( putWritten >= putToWrite ) {
	dataSocket->close();
	QTimer::singleShot( 1, this, SLOT( dataClosed() ) );
    }
}

/*!  \internal
  Reinitializes the command socket
*/

void QFtp::reinitCommandSocket()
{
}

/*!  \reimp
*/

void QFtp::error( int code )
{
    if ( code == QSocket::ErrHostNotFound ||
	 code == QSocket::ErrConnectionRefused ) {
	if ( dataSocket->isOpen() )
	    dataSocket->close();
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    QString msg;
	    if ( code == QSocket::ErrHostNotFound )
		msg = tr( "Host %1 not found" ).arg( url()->host() );
	    else
		msg = tr( "Connection refused to host %1" ).arg( url()->host() );
	    op->setState( StFailed );
	    op->setProtocolDetail( msg );
	    op->setErrorCode( (int)ErrHostNotFound );
	}
    }
}





/////////////////////////////////////////////////
// new stuff
//

#include "qftp.moc"

//
//  end of new stuff
/////////////////////////////////////////////////


#endif // QT_NO_NETWORKPROTOCOL_FTP
