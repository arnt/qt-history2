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
//#define QFTPDTP_DEBUG

/*!
    \class QFtp qftp.h
    \brief The QFtp class provides an implementation of the FTP protocol.

    \ingroup io
    \module network

    This class is derived from QNetworkProtocol. QFtp is not normally
    used directly, but rather through a QUrlOperator, for example:

    \code
    QUrlOperator op( "ftp://ftp.trolltech.com" );
    op.listChildren(); // Asks the server to provide a directory listing
    \endcode

    This code will only work if the QFtp class is registered; to
    register the class, you must call qInitNetworkProtocols() before
    using a QUrlOperator with QFtp.

    If you really need to use QFtp directly, don't forget to set its
    QUrlOperator with setUrl().

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

class QFtpPI;

class QFtpDTP : public QObject
{
    Q_OBJECT

public:
    enum ConnectState {
	CsHostFound,
	CsConnected,
	CsClosed,
	CsHostNotFound,
	CsConnectionRefused
    };

    QFtpDTP( QFtpPI *p, QObject *parent=0, const char *name=0 );

    void setData( QByteArray * );
    void setDevice( QIODevice * );
    void writeData();

    void setBytesTotal( int bytes )
    {
	bytesTotal = bytes;
	bytesDone = 0;
	emit dataTransferProgress( bytesDone, bytesTotal );
    }

    bool hasError() const;
    QString errorMessage() const;
    void clearError();

    void connectToHost( const QString & host, Q_UINT16 port )
    { socket.connectToHost( host, port ); }

    QSocket::State socketState() const
    { return socket.state(); }

    void abortConnection();

    static bool parseDir( const QString &buffer, const QString &userName, QUrlInfo *info );

signals:
    void listInfo( const QUrlInfo& );
    void newData( const QByteArray& );
    void dataTransferProgress( int, int );

    void connectState( int );

private slots:
    void socketConnected();
    void socketReadyRead();
    void socketError( int );
    void socketConnectionClosed();
    void socketBytesWritten( int );

private:
    void clearData()
    {
	data_ba = FALSE;
	data.ba = 0;
    }

    QSocket socket;
    QFtpPI *pi;
    QString err;
    int bytesDone;
    int bytesTotal;
    bool callWriteData;

    union {
	QByteArray *ba;
	QIODevice *dev;
    } data;
    bool data_ba;
};

class QFtpPI : public QObject
{
    Q_OBJECT

public:
    QFtpPI( QObject *parent=0 );

    void connectToHost( const QString &host, Q_UINT16 port );

    bool sendCommands( const QStringList &cmds );
    bool sendCommand( const QString &cmd )
    { return sendCommands( QStringList( cmd ) ); }

    void clearPendingCommands();
    void abort();

    QString currentCommand() const
    { return currentCmd; }

    QFtpDTP dtp; // the PI has a DTP which is not the design of RFC 959, but it
		 // makes the design simpler this way
signals:
    void connectState( int );
    void finished( const QString& );
    void error( int, const QString& );
    void rawFtpReply( int, const QString& );

private slots:
    void hostFound();
    void connected();
    void connectionClosed();
    void delayedCloseFinished();
    void readyRead();
    void error( int );

    void dtpConnectState( int );

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

    enum AbortState {
	None,
	AbortStarted,
	WaitForAbortToFinish
    };

    bool processReply();
    bool startNextCmd();

    QSocket commandSocket;
    QString replyText;
    char replyCode[3];
    State state;
    AbortState abortState;
    QStringList pendingCommands;
    QString currentCmd;

    bool waitForDtpToConnect;
    bool waitForDtpToClose;
};

class QFtpCommand
{
public:
    QFtpCommand( QFtp::Command cmd, QStringList raw );
    QFtpCommand( QFtp::Command cmd, QStringList raw, const QByteArray &ba );
    QFtpCommand( QFtp::Command cmd, QStringList raw, QIODevice *dev );
    ~QFtpCommand();

    int id;
    QFtp::Command command;
    QStringList rawCmds;
    union {
	QByteArray *ba;
	QIODevice *dev;
    } data;
    bool data_ba;

    static int idCounter;
};

/**********************************************************************
 *
 * QFtpCommand implemenatation
 *
 *********************************************************************/
int QFtpCommand::idCounter = 0;

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw )
    : command(cmd), rawCmds(raw), data_ba(FALSE)
{
    id = ++idCounter;
    data.ba = 0;
}

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw, const QByteArray &ba )
    : command(cmd), rawCmds(raw), data_ba(TRUE)
{
    id = ++idCounter;
    data.ba = new QByteArray( ba );
}

QFtpCommand::QFtpCommand( QFtp::Command cmd, QStringList raw, QIODevice *dev )
    : command(cmd), rawCmds(raw), data_ba(FALSE)
{
    id = ++idCounter;
    data.dev = dev;
}

QFtpCommand::~QFtpCommand()
{
    if ( data_ba )
	delete data.ba;
}

/**********************************************************************
 *
 * QFtpDTP implemenatation
 *
 *********************************************************************/
QFtpDTP::QFtpDTP( QFtpPI *p, QObject *parent, const char *name ) :
    QObject( parent, name ), pi( p ),
    err( QString::null ), callWriteData( FALSE )
{
    clearData();

    connect( &socket, SIGNAL( connected() ),
	     SLOT( socketConnected() ) );
    connect( &socket, SIGNAL( readyRead() ),
	     SLOT( socketReadyRead() ) );
    connect( &socket, SIGNAL( error( int ) ),
	     SLOT( socketError( int ) ) );
    connect( &socket, SIGNAL( connectionClosed() ),
	     SLOT( socketConnectionClosed() ) );
    connect( &socket, SIGNAL( bytesWritten( int ) ),
	     SLOT( socketBytesWritten( int ) ) );
}

void QFtpDTP::setData( QByteArray *ba )
{
    data_ba = TRUE;
    data.ba = ba;
}

void QFtpDTP::setDevice( QIODevice *dev )
{
    data_ba = FALSE;
    data.dev = dev;
}

void QFtpDTP::writeData()
{
    if ( data_ba ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::writeData: write %d bytes", data.ba->size() );
#endif
	if ( data.ba->size() == 0 )
	    emit dataTransferProgress( 0, bytesTotal );
	else
	    socket.writeBlock( data.ba->data(), data.ba->size() );
	socket.close();
	clearData();
    } else if ( data.dev ) {
	callWriteData = FALSE;
	const int blockSize = 16*1024;
	char buf[blockSize];
	while ( !data.dev->atEnd() && socket.bytesToWrite()==0 ) {
	    Q_LONG read = data.dev->readBlock( buf, blockSize );
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP::writeData: writeBlock() of size %d bytes", (int)read );
#endif
	    socket.writeBlock( buf, read );
	    if ( !data.dev )
		return; // this can happen when a command is aborted
	}
	if ( data.dev->atEnd() ) {
	    if ( bytesDone==0 && socket.bytesToWrite()==0 )
		emit dataTransferProgress( 0, bytesTotal );
	    socket.close();
	    clearData();
	} else {
	    callWriteData = TRUE;
	}
    }
}

inline bool QFtpDTP::hasError() const
{
    return !err.isNull();
}

inline QString QFtpDTP::errorMessage() const
{
    return err;
}

inline void QFtpDTP::clearError()
{
    err = QString::null;
}

void QFtpDTP::abortConnection()
{
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::abortConnection" );
#endif
    callWriteData = FALSE;
    clearData();

    socket.clearPendingData();
    socket.close();
}

bool QFtpDTP::parseDir( const QString &buffer, const QString &userName, QUrlInfo *info )
{
    QStringList lst = QStringList::split( " ", buffer );

    if ( lst.count() < 9 )
	return FALSE;

    QString tmp;

    // permissions
    tmp = lst[ 0 ];

    if ( tmp[ 0 ] == QChar( 'd' ) ) {
	info->setDir( TRUE );
	info->setFile( FALSE );
	info->setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( '-' ) ) {
	info->setDir( FALSE );
	info->setFile( TRUE );
	info->setSymLink( FALSE );
    } else if ( tmp[ 0 ] == QChar( 'l' ) ) {
	info->setDir( TRUE ); // #### todo
	info->setFile( FALSE );
	info->setSymLink( TRUE );
    } else {
	return FALSE;
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
    info->setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info->setGroup( tmp );

    // ### not correct
    info->setWritable( ( userName == info->owner() && perms[ user ][ writable ] ) ||
	perms[ other ][ writable ] );
    info->setReadable( ( userName == info->owner() && perms[ user ][ readable ] ) ||
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
    info->setPermissions( p );

    // size
    tmp = lst[ 4 ];
    info->setSize( tmp.toInt() );

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
    info->setLastModified( QDateTime( date, time ) );

    if ( lst[ 7 ].contains( ":" ) ) {
	if( info->lastModified() > QDateTime::currentDateTime() ) {
	    QDateTime dt = info->lastModified();
	    QDate d = dt.date();
	    d.setYMD(d.year()-1, d.month(), d.day());
	    dt.setDate(d);
	    info->setLastModified(dt);
	}
    }

    // name
    if ( info->isSymLink() )
	info->setName( lst[ 8 ].stripWhiteSpace() );
    else {
	QString n;
	for ( uint i = 8; i < lst.count(); ++i )
	    n += lst[ i ] + " ";
	n = n.stripWhiteSpace();
	info->setName( n );
    }
    return TRUE;
}

void QFtpDTP::socketConnected()
{
    bytesDone = 0;
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::connectState( CsConnected )" );
#endif
    emit connectState( QFtpDTP::CsConnected );
}

void QFtpDTP::socketReadyRead()
{
    if ( pi->currentCommand().isEmpty() ) {
	socket.close();
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsClosed )" );
#endif
	emit connectState( QFtpDTP::CsClosed );
	return;
    }

    if ( pi->currentCommand().startsWith("LIST") ) {
	while ( socket.canReadLine() ) {
	    QUrlInfo i;
	    QString line = socket.readLine();
#if defined(QFTPDTP_DEBUG)
	    qDebug( "QFtpDTP read (list): '%s'", line.latin1() );
#endif
	    if ( parseDir( line, "", &i ) ) {
		emit listInfo( i );
	    } else {
		// some FTP servers don't return a 550 if the file or directory
		// does not exist, but rather write a text to the data socket
		// -- try to catch these cases
		if ( line.endsWith( "No such file or directory\r\n" ) )
		    err = line;
	    }
	}
    } else {
	QByteArray ba( socket.bytesAvailable() );
	Q_LONG bytesRead = socket.readBlock( ba.data(), ba.size() );
	if ( bytesRead < 0 ) {
	    // ### error handling
	    return;
	}
	ba.resize( bytesRead );
	bytesDone += bytesRead;
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP read: %d bytes (total %d bytes)", (int)bytesRead, bytesDone );
#endif
	emit dataTransferProgress( bytesDone, bytesTotal );
	if ( !data_ba && data.dev )
	    data.dev->writeBlock( ba );
	else
	    emit newData( ba );
    }
}

void QFtpDTP::socketError( int e )
{
    if ( e == QSocket::ErrHostNotFound ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsHostNotFound )" );
#endif
	emit connectState( QFtpDTP::CsHostNotFound );
    } else if ( e == QSocket::ErrConnectionRefused ) {
#if defined(QFTPDTP_DEBUG)
	qDebug( "QFtpDTP::connectState( CsConnectionRefused )" );
#endif
	emit connectState( QFtpDTP::CsConnectionRefused );
    }
}

void QFtpDTP::socketConnectionClosed()
{
    if ( !data_ba && data.dev ) {
	clearData();
    }
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::connectState( CsClosed )" );
#endif
    emit connectState( QFtpDTP::CsClosed );
}

void QFtpDTP::socketBytesWritten( int bytes )
{
    bytesDone += bytes;
#if defined(QFTPDTP_DEBUG)
    qDebug( "QFtpDTP::bytesWritten( %d )", bytesDone );
#endif
    emit dataTransferProgress( bytesDone, bytesTotal );
    if ( callWriteData )
	writeData();
}

/**********************************************************************
 *
 * QFtpPI implemenatation
 *
 *********************************************************************/
QFtpPI::QFtpPI( QObject *parent ) :
    QObject( parent ),
    dtp(this),
    state(Begin), abortState(None),
    currentCmd(QString::null),
    waitForDtpToConnect(FALSE),
    waitForDtpToClose(FALSE)
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

    connect( &dtp, SIGNAL(connectState(int)),
	     SLOT(dtpConnectState(int)) );
}

void QFtpPI::connectToHost( const QString &host, Q_UINT16 port )
{
    emit connectState( QFtp::HostLookup );
    commandSocket.connectToHost( host, port );
}

/*
  Sends the sequence of commands \a cmds to the FTP server. When the commands
  are all done the finished() signal is emitted. When an error occurs, the
  error() signal is emitted.

  If there are pending commands in the queue this functions returns FALSE and
  the \a cmds are not added to the queue; otherwise it returns TRUE.
*/
bool QFtpPI::sendCommands( const QStringList &cmds )
{
    if ( !pendingCommands.isEmpty() )
	return FALSE;

    if ( commandSocket.state()!=QSocket::Connected || state!=Idle ) {
	emit error( QFtp::NotConnected, tr( "Not connected" ) );
	return TRUE; // there are no pending commands
    }

    pendingCommands = cmds;
    startNextCmd();
    return TRUE;
}

void QFtpPI::clearPendingCommands()
{
    pendingCommands.clear();
    currentCmd = QString::null;
    state = Idle;
}

void QFtpPI::abort()
{
    pendingCommands.clear();

    if ( abortState != None )
	// ABOR already sent
	return;

    abortState = AbortStarted;
#if defined(QFTPPI_DEBUG)
    qDebug( "QFtpPI send: ABOR" );
#endif
    commandSocket.writeBlock( "ABOR\r\n", 6 );

    if ( currentCmd.startsWith("STOR ") )
	dtp.abortConnection();
}

void QFtpPI::hostFound()
{
    emit connectState( QFtp::Connecting );
}

void QFtpPI::connected()
{
    state = Begin;
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [connected()]", state );
#endif
    emit connectState( QFtp::Connected );
}

void QFtpPI::connectionClosed()
{
    commandSocket.close();
    emit connectState( QFtp::Unconnected );
}

void QFtpPI::delayedCloseFinished()
{
    emit connectState( QFtp::Unconnected );
}

void QFtpPI::error( int e )
{
    if ( e == QSocket::ErrHostNotFound ) {
	emit connectState( QFtp::Unconnected );
	emit error( QFtp::HostNotFound,
		tr( "Host %1 not found" ).arg( commandSocket.peerName() ) );
    } else if ( e == QSocket::ErrConnectionRefused ) {
	emit connectState( QFtp::Unconnected );
	emit error( QFtp::ConnectionRefused,
		tr( "Connection refused to host %1" ).arg( commandSocket.peerName() ) );
    }
}

void QFtpPI::readyRead()
{
    if ( waitForDtpToClose )
	return;

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
		replyCode[i] = line[i].digitValue();
		if ( replyCode[i]<lowerLimit[i] || replyCode[i]>upperLimit[i] ) {
		    // ### protocol error
		    return;
		}
	    }
	}
	QString endOfMultiLine;
	endOfMultiLine[0] = '0' + replyCode[0];
	endOfMultiLine[1] = '0' + replyCode[1];
	endOfMultiLine[2] = '0' + replyCode[2];
	endOfMultiLine[3] = ' ';
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

	if ( processReply() )
	    replyText = "";
    }
}

/*
  Process a reply from the FTP server.

  Returns TRUE if the reply was processed or FALSE if the reply has to be
  processed at a later point.
*/
bool QFtpPI::processReply()
{
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() begin]", state );
    if ( replyText.length() < 400 )
	qDebug( "QFtpPI recv: %d %s", 100*replyCode[0]+10*replyCode[1]+replyCode[2], replyText.latin1() );
    else
	qDebug( "QFtpPI recv: %d (text skipped)", 100*replyCode[0]+10*replyCode[1]+replyCode[2] );
#endif

    // process 226 replies ("Closing Data Connection") only when the data
    // connection is really closed to avoid short reads of the DTP
    if ( 100*replyCode[0]+10*replyCode[1]+replyCode[2] == 226 ) {
	if ( dtp.socketState() != QSocket::Idle ) {
	    waitForDtpToClose = TRUE;
	    return FALSE;
	}
    }

    switch ( abortState ) {
	case AbortStarted:
	    abortState = WaitForAbortToFinish;
	    break;
	case WaitForAbortToFinish:
	    abortState = None;
	    return TRUE;
	default:
	    break;
    }

    // get new state
    static const State table[5] = {
	/* 1yz   2yz      3yz   4yz      5yz */
	Waiting, Success, Idle, Failure, Failure
    };
    switch ( state ) {
	case Begin:
	    if ( replyCode[0] == 1 ) {
		return TRUE;
	    } else if ( replyCode[0] == 2 ) {
		state = Idle;
		emit finished( tr( "Connected to host %1" ).arg( commandSocket.peerName() ) );
		break;
	    }
	    // ### error handling
	    return TRUE;
	case Waiting:
	    if ( replyCode[0]<0 || replyCode[0]>5 )
		state = Failure;
	    else
		state = table[ replyCode[0] - 1 ];
	    break;
	default:
	    // ### spontaneous message
	    return TRUE;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() intermediate]", state );
#endif

    // special actions on certain replies
    int replyCodeInt = 100*replyCode[0] + 10*replyCode[1] + replyCode[2];
    emit rawFtpReply( replyCodeInt, replyText );
    if ( replyCodeInt == 227 ) {
	// 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
	int l = replyText.find( "(" );
	int r = replyText.find( ")" );
	if ( l<0 || r<0 ) {
#if defined(QFTPPI_DEBUG)
	    qDebug( "QFtp: bad 227 response -- address and port information missing" );
#endif
	    // ### error handling
	} else {
	    QStringList lst = QStringList::split( ',', replyText.mid(l+1,r-l-1) );
	    QString host = lst[0] + "." + lst[1] + "." + lst[2] + "." + lst[3];
	    Q_UINT16 port = ( lst[4].toUInt() << 8 ) + lst[5].toUInt();
	    waitForDtpToConnect = TRUE;
	    dtp.connectToHost( host, port );
	}
    } else if ( replyCodeInt == 230 ) {
	// 230 User logged in, proceed.
	emit connectState( QFtp::LoggedIn );
    } else if ( replyCodeInt == 213 ) {
	// 213 File status.
	if ( currentCmd.startsWith("SIZE ") )
	    dtp.setBytesTotal( replyText.simplifyWhiteSpace().toInt() );
    } else if ( replyCode[0]==1 && currentCmd.startsWith("STOR ") ) {
	dtp.writeData();
    }

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
	    if ( dtp.hasError() ) {
		emit error( QFtp::UnknownError, dtp.errorMessage() );
		dtp.clearError();
	    }
	    startNextCmd();
	    break;
	case Waiting:
	    // ### do nothing
	    break;
	case Failure:
	    emit error( QFtp::UnknownError, replyText );
	    state = Idle;
	    startNextCmd();
	    break;
    }
#if defined(QFTPPI_DEBUG)
//    qDebug( "QFtpPI state: %d [processReply() end]", state );
#endif
    return TRUE;
}

/*
  Starts next pending command. Returns FALSE if there are no pending commands,
  otherwise it returns TRUE.
*/
bool QFtpPI::startNextCmd()
{
    if ( waitForDtpToConnect )
	// don't process any new commands until we are connected
	return TRUE;

#if defined(QFTPPI_DEBUG)
    if ( state != Idle )
	qDebug( "QFtpPI startNextCmd: Internal error! QFtpPI called in non-Idle state %d", state );
#endif
    if ( pendingCommands.isEmpty() ) {
	currentCmd = QString::null;
	emit finished( replyText );
	return FALSE;
    }
    currentCmd = pendingCommands.first();
    pendingCommands.pop_front();
#if defined(QFTPPI_DEBUG)
    qDebug( "QFtpPI send: %s", currentCmd.left( currentCmd.length()-2 ).latin1() );
#endif
    state = Waiting;
    commandSocket.writeBlock( currentCmd.latin1(), currentCmd.length() );
    return TRUE;
}

void QFtpPI::dtpConnectState( int s )
{
    switch ( s ) {
	case QFtpDTP::CsClosed:
	    if ( waitForDtpToClose ) {
		// there is an unprocessed reply
		if ( processReply() )
		    replyText = "";
		else
		    return;
	    }
	    waitForDtpToClose = FALSE;
	    readyRead();
	    return;
	case QFtpDTP::CsConnected:
	    waitForDtpToConnect = FALSE;
	    startNextCmd();
	    return;
	case QFtpDTP::CsHostNotFound:
	case QFtpDTP::CsConnectionRefused:
	    emit error( QFtp::ConnectionRefused,
		    tr( "Connection refused for data connection" ) );
	    startNextCmd();
	    return;
	default:
	    return;
    }
}

/**********************************************************************
 *
 * QFtpPrivate
 *
 *********************************************************************/
class QFtpPrivate
{
public:
    QFtpPrivate() :
	close_waitForStateChange(FALSE),
	state( QFtp::Unconnected ),
	error( QFtp::NoError )
    { pending.setAutoDelete( TRUE ); }

    QFtpPI pi;
    QPtrList<QFtpCommand> pending;
    bool close_waitForStateChange;
    QFtp::State state;
    QFtp::Error error;
    QString errorString;
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

/**********************************************************************
 *
 * QFtp implementation
 *
 *********************************************************************/
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
    d->errorString = tr( "Unknown error" );

    connect( &d->pi, SIGNAL(connectState(int)),
	    SLOT(piConnectState(int)) );
    connect( &d->pi, SIGNAL(finished( const QString& )),
	    SLOT(piFinished( const QString& )) );
    connect( &d->pi, SIGNAL(error(int, const QString&)),
	    SLOT(piError(int, const QString&)) );
    connect( &d->pi, SIGNAL(rawFtpReply(int, const QString&)),
	    SLOT(piFtpReply(int, const QString&)) );

    connect( &d->pi.dtp, SIGNAL(newData(const QByteArray&)),
	    SIGNAL(newData(const QByteArray&)) );
    connect( &d->pi.dtp, SIGNAL(dataTransferProgress(int,int)),
	    SIGNAL(dataTransferProgress(int,int)) );
    connect( &d->pi.dtp, SIGNAL(listInfo(const QUrlInfo&)),
	    SIGNAL(listInfo(const QUrlInfo&)) );
}

/*!
  \enum QFtp::State

  This enum defines the changes of the connection state:

  \value Unconnected if no connection to the host exists
  \value HostLookup if it started a host name lookup
  \value Connecting if it tries to connect to the host
  \value Connected if a connection to the host exists
  \value LoggedIn if a connection to the host exists and the user is logged in
  \value Closing if the connection is closing down, but is not yet closed

  \sa stateChanged() state()
*/
/*!
  \enum QFtp::Command
###
*/
/*!  \fn void QFtp::stateChanged( int state )
  This signal is emitted when the state of the connection changes. The argument
  \a state is the new state of the connection; it is one of the enum \l
  State values.

  It is usually emitted in reaction of a connectToHost() or close() command,
  but it can also be emitted "spontaneous", e.g. when the server closes the
  connection unexpectedly.

  \sa connectToHost() close() state() State
*/
/*!  \fn void QFtp::listInfo( const QUrlInfo &i );
  This signal is emitted for each directory entry the list() command can find.
  The details of the entry are stored in \a i.

  \sa list()
*/
/*!  \fn void QFtp::commandStarted( int id )
  This signal is emitted ###
*/
/*!  \fn void QFtp::commandFinished( int id, bool error )
  This signal is emitted ###
*/
/*!  \fn void QFtp::done( bool error )
  This signal is emitted ###
*/
/*!  \fn void QFtp::newData( const QByteArray &data )
  This signal is emitted ###
*/
/*!  \fn void QFtp::dataTransferProgress( int bytesDone, int bytesTotal )
  This signal is emitted ###
*/
/*!  \fn void QFtp::rawCommandReply( int replyCode, const QString &detail );
    This signal is emitted in reply to the rawCommand() function. \a replyCode
    is the 3 digit reply code and \a detail is the textthat follows the reply
    code.

    \sa rawCommand()
*/

/*!
  Connects to the FTP server \a host at the \a port. This function returns
  immediately and does not block until the connection succeeded. The
  stateChanged() signal is emitted when the state of the connecting
  process changes.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::connectToHost( const QString &host, Q_UINT16 port )
{
    QStringList cmds;
    cmds << host;
    cmds << QString::number( (uint)port );
    return addCommand( new QFtpCommand( ConnectToHost, cmds ) );
}

/*!
  Logins to the FTP server with the username \a user and the password \a
  password.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa commandStarted() commandFinished()
*/
int QFtp::login( const QString &user, const QString &password )
{
    QStringList cmds;
    cmds << "USER " + ( user.isNull() ? QString("anonymous") : user ) + "\r\n";
    cmds << "PASS " + ( password.isNull() ? QString("anonymous@") : password ) + "\r\n";
    return addCommand( new QFtpCommand( Login, cmds ) );
}

/*!
  Closes the connection to the FTP server.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa stateChanged() commandStarted() commandFinished()
*/
int QFtp::close()
{
    return addCommand( new QFtpCommand( Close, QStringList("QUIT\r\n") ) );
}

/*!
  Lists the directory content of the directory \a dir of the FTP server. If
  \a dir is empty, it lists the content of the current directory.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa listInfo() commandStarted() commandFinished()
*/
int QFtp::list( const QString &dir )
{
    QStringList cmds;
    cmds << "TYPE A\r\n";
    cmds << "PASV\r\n";
    if ( dir.isEmpty() )
	cmds << "LIST\r\n";
    else
	cmds << "LIST " + dir + "\r\n";
    return addCommand( new QFtpCommand( List, cmds ) );
}

/*!
  Changes the working directory of the server to \a dir.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa commandStarted() commandFinished()
*/
int QFtp::cd( const QString &dir )
{
    return addCommand( new QFtpCommand( Cd, QStringList("CWD "+dir+"\r\n") ) );
}

/*!
  Downloads the file \a file from the server. The downloaded file is reported
  in chunks by the newData() signal.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa newData() dataTransferProgress() commandStarted() commandFinished()
*/
int QFtp::get( const QString &file )
{
    QStringList cmds;
    cmds << "SIZE " + file + "\r\n";
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    cmds << "RETR " + file + "\r\n";
    return addCommand( new QFtpCommand( Get, cmds ) );
}

/*! \overload
  Downloads the file \a file from the server and stores it on the IO device \a
  dev. Make sure that the \a dev pointer is valid throughout the whole pending
  operation (it is safe to emit it when the commandFinished() is emitted).

  This overload emits the same signals, except for the newData() signal which
  is never emitted.
*/
int QFtp::get( const QString &file, QIODevice *dev )
{
    QStringList cmds;
    cmds << "SIZE " + file + "\r\n";
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    cmds << "RETR " + file + "\r\n";
    return addCommand( new QFtpCommand( Get, cmds, dev ) );
}

/*! \overload
  Stores the data \a data under \a file on the server. The progress of the
  upload is reported by the dataTransferProgress() signal.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa dataTransferProgress() commandStarted() commandFinished()
*/
int QFtp::put( const QByteArray &data, const QString &file )
{
    QStringList cmds;
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    cmds << "ALLO " + QString::number(data.size()) + "\r\n";
    cmds << "STOR " + file + "\r\n";
    return addCommand( new QFtpCommand( Put, cmds, data ) );
}

/*!
  Reads the data from the IO device \a dev and stores it under \a file on the
  server. The data is read in chunks from the IO device, so this overload
  allows you to transmit large amounts of data without the need to read all
  data into memory at once.

  Make sure that the \a dev pointer is valid throughout the whole pending
  operation (it is safe to emit it when the commandFinished() is emitted).
*/
int QFtp::put( QIODevice *dev, const QString &file )
{
    QStringList cmds;
    cmds << "TYPE I\r\n";
    cmds << "PASV\r\n";
    if ( !dev->isSequentialAccess() )
	cmds << "ALLO " + QString::number(dev->size()) + "\r\n";
    cmds << "STOR " + file + "\r\n";
    return addCommand( new QFtpCommand( Put, cmds, dev ) );
}

/*!
  Deletes the file \a file from the server.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa commandStarted() commandFinished()
*/
int QFtp::remove( const QString &file )
{
    return addCommand( new QFtpCommand( Remove, QStringList("DELE "+file+"\r\n") ) );
}

/*!
  Creates the directory \a dir on the server.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa commandStarted() commandFinished()
*/
int QFtp::mkdir( const QString &dir )
{
    return addCommand( new QFtpCommand( Mkdir, QStringList("MKD "+dir+"\r\n") ) );
}

/*!
  Removes the directory \a dir from the server.

  This function returns immediately; the command is scheduled and its execution
  is done asynchronous. In order to identify this command, the function returns
  a unique identifier.

  When the command is started the commandStarted() signal is emitted. When it is
  finished, either the commandFinished() signal is emitted.

  \sa commandStarted() commandFinished()
*/
int QFtp::rmdir( const QString &dir )
{
    return addCommand( new QFtpCommand( Rmdir, QStringList("RMD "+dir+"\r\n") ) );
}

/*!
    Sends the raw FTP command \a command to the FTP server. This is useful for
    low-level FTP access; if there is a QFtp function for the FTP command you
    want to issue, it is in general easier and safer to use that one instead.

    This function returns immediately; the command is scheduled and its
    execution is done asynchronous. In order to identify this command, the
    function returns a unique identifier.

    When the command is started the commandStarted() signal is emitted. When it is
    finished, either the commandFinished() signal is
    emitted.

    \sa rawCommandReply() commandStarted() commandFinished()
*/
int QFtp::rawCommand( const QString &command )
{
    QString cmd = command.stripWhiteSpace() + "\r\n";
    return addCommand( new QFtpCommand( FtpCommand, QStringList(cmd) ) );
}

/*!
  Aborts the current command and deletes all scheduled commands.

  If there is a started but not finished command (i.e. a command for which the
  commandStarted() signal was already emitted, but no commandFinished()
  signal is emitted yet), this function sends an \c ABORT command to the
  server. When the server replies that the command is aborted, the
  commandFinished() signal with \c FALSE is emitted for that command. Due to
  timing issues, it is possible that the command is already finished before the
  abort request arrives at the server. In this case, the commandFinished()
  signal is emitted.

  For all other commands that are affected by the abort(), no signals are
  emitted.

  If you don't start further FTP commands directly after the abort(), there
  won't be any scheduled commands and the done() signal is emitted.

  \warning We experienced that some FTP servers, namely the BSD FTP daemon
  (version 0.3), wrongly return a positive reply in the case that the abort was
  successful and as a result the commandFinished() signal is emitted, although
  the command was aborted.

  \sa clearPendingCommands()
*/
void QFtp::abort()
{
    QFtpPrivate *d = ::d( this );
    if ( d->pending.isEmpty() )
	return;

    clearPendingCommands();
    d->pi.abort();
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

/*!
    Returns TRUE if there are any commands scheduled, but not executed yet;
    otherwise it returns FALSE.

    The command that is currently executed is not considered as a scheduled
    command.

    \sa clearPendingCommands() currentId() currentCommand()
*/
bool QFtp::hasPendingCommands() const
{
    QFtpPrivate *d = ::d( this );
    return d->pending.count() > 1;
}

/*!
    Deletes all pending commands from the list of scheduled commands. This does
    not affect the command that is currently executed. If you want to stop this
    this as well, use abort().

    \sa hasPendingCommands() abort()
*/
void QFtp::clearPendingCommands()
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.take( 0 );
    d->pending.clear();
    d->pending.append( c );
}

/*!
  Returns the current state of the object. When the state changes, the
  stateChanged() signal is emitted.

  \sa State stateChanged()
*/
QFtp::State QFtp::state() const
{
    QFtpPrivate *d = ::d( this );
    return d->state;
}

/*!
    Returns the last error that occured. This is useful to find out details
    about the failure when receiving a commandFinished() or a done() signal
    with the \c error argument \c FALSE.

    If you start a new command, the error status is reset to \c NoError.
*/
QFtp::Error QFtp::error() const
{
    QFtpPrivate *d = ::d( this );
    return d->error;
}

/*!
    Returns a human readable description of the last error that occured. This
    is useful to present a error message to the user when receiving a
    commandFinished() or a done() signal with the \c error argument \c FALSE.

    The error string is often (but not always) the reply from the server. In
    such a case, it is not possible to translate the string. If the message
    comes from Qt, the strings are already sent through tr().
*/
QString QFtp::errorString() const
{
    QFtpPrivate *d = ::d( this );
    return d->errorString;
}


int QFtp::addCommand( QFtpCommand *cmd )
{
    QFtpPrivate *d = ::d( this );
    d->pending.append( cmd );

    if ( d->pending.count() == 1 )
	// don't emit the commandStarted() signal before the id is returned
	QTimer::singleShot( 0, this, SLOT(startNextCommand()) );

    return cmd->id;
}

void QFtp::startNextCommand()
{
    QFtpPrivate *d = ::d( this );

    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;

    d->error = NoError;
    d->errorString = tr( "Unknown error" );

    emit commandStarted( c->id );

    if ( c->command == ConnectToHost ) {
	d->pi.connectToHost( c->rawCmds[0], c->rawCmds[1].toUInt() );
    } else {
	if ( c->command == Put ) {
	    if ( c->data_ba && c->data.ba ) {
		d->pi.dtp.setData( c->data.ba );
		d->pi.dtp.setBytesTotal( c->data.ba->size() );
	    } else if ( !c->data_ba && c->data.dev ) {
		d->pi.dtp.setDevice( c->data.dev );
		if ( c->data.dev->isSequentialAccess() )
		    d->pi.dtp.setBytesTotal( 0 );
		else
		    d->pi.dtp.setBytesTotal( c->data.dev->size() );
	    }
	} else if ( c->command == Get ) {
	    if ( !c->data_ba && c->data.dev ) {
		d->pi.dtp.setDevice( c->data.dev );
	    }
	} else if ( c->command == Close ) {
	    d->state = QFtp::Closing;
	    emit stateChanged( d->state );
	}
	if ( !d->pi.sendCommands( c->rawCmds ) ) {
	    // ### error handling (this case should not happen)
	}
    }
}

void QFtp::piFinished( const QString& )
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();
    if ( c == 0 )
	return;

    if ( c->command == Close ) {
	// The order of in which the slots are called is arbitrary, so
	// disconnect the SIGNAL-SIGNAL temporary to make sure that we
	// don't get the commandFinished() signal before the stateChanged()
	// signal.
	if ( d->state != QFtp::Unconnected ) {
	    d->close_waitForStateChange = TRUE;
	    return;
	}
    }
    emit commandFinished( c->id, FALSE );

    d->pending.removeFirst();
    if ( d->pending.isEmpty() )
	emit done( FALSE );
    else
	startNextCommand();
}

void QFtp::piError( int errorCode, const QString &text )
{
    QFtpPrivate *d = ::d( this );
    QFtpCommand *c = d->pending.getFirst();

    // non-fatal errors
    if ( c->command==Get && d->pi.currentCommand().startsWith("SIZE ") ) {
	d->pi.dtp.setBytesTotal( -1 );
	return;
    } else if ( c->command==Put && d->pi.currentCommand().startsWith("ALLO ") ) {
	return;
    }

    d->error = (Error)errorCode;
    switch ( currentCommand() ) {
	case ConnectToHost:
	    d->errorString = tr( "Connecting to host failed:\n%1" ).arg( text );
	    break;
	case Login:
	    d->errorString = tr( "Login failed:\n%1" ).arg( text );
	    break;
	case List:
	    d->errorString = tr( "Listing directory failed:\n%1" ).arg( text );
	    break;
	case Cd:
	    d->errorString = tr( "Changing directory failed:\n%1" ).arg( text );
	    break;
	case Get:
	    d->errorString = tr( "Downloading file failed:\n%1" ).arg( text );
	    break;
	case Put:
	    d->errorString = tr( "Uploading file failed:\n%1" ).arg( text );
	    break;
	case Remove:
	    d->errorString = tr( "Removing file failed:\n%1" ).arg( text );
	    break;
	case Mkdir:
	    d->errorString = tr( "Creating directory failed:\n%1" ).arg( text );
	    break;
	case Rmdir:
	    d->errorString = tr( "Removing directory failed:\n%1" ).arg( text );
	    break;
	default:
	    d->errorString = text;
	    break;
    }

    d->pi.clearPendingCommands();
    emit commandFinished( c->id, TRUE );

    d->pending.clear();
    emit done( TRUE );

}

void QFtp::piConnectState( int state )
{
    QFtpPrivate *d = ::d( this );
    d->state = (State)state;
    emit stateChanged( d->state );
    if ( d->close_waitForStateChange ) {
	d->close_waitForStateChange = FALSE;
	piFinished( tr( "Connection closed" ) );
    }
}

void QFtp::piFtpReply( int code, const QString &text )
{
    if ( currentCommand() == FtpCommand )
	emit rawCommandReply( code, text );
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

    closeInternal();
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

    switch ( operationInProgress()->operation() )
    {
	case OpGet:
	case OpPut:
	    {
		QUrl u( operationInProgress()->arg( 0 ) );
		commandSocket->connectToHost( u.host(),
			u.port() != -1 ? u.port() : 21 );
	    }
	    break;
	default:
	    commandSocket->connectToHost( url()->host(),
		    url()->port() != -1 ? url()->port() : 21 );
	    break;
    }

#if defined(QFTP_DEBUG)
	qDebug( "QFtp: try connecting!" );
#endif
    return FALSE;
}

/*!
    Closes the command and data connections to the FTP server
*/

void QFtp::closeInternal()
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
    Parses the string, \a buffer, which is one line of a directory
    listing which came from the FTP server, and sets the values which
    have been parsed to the url info object, \a info.
*/

void QFtp::parseDir( const QString &buffer, QUrlInfo &info )
{
    QFtpDTP::parseDir( buffer, url()->user(), &info );
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
    If data has arrived on the command socket, this slot is called.
    The function looks at the data and passes it on to an appropriate
    handler function.
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
	if ( putToWrite == 0 )
	    dataBytesWritten( 0 );
	else
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
	closeInternal();
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
    Some operations require a data connection to the server. If this
    connection could be opened, this function handles the data
    connection.
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
	emit QNetworkProtocol::dataTransferProgress( 0, getTotalSize, operationInProgress() );
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
    This function is called when new data has arrived on the data socket.
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
	emit QNetworkProtocol::dataTransferProgress( getDoneSize, getTotalSize, operationInProgress() );
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
    This function is called when \a nbytes have been successfully
    written to the data socket.
*/

void QFtp::dataBytesWritten( int nbytes )
{
    putWritten += nbytes;
    emit QNetworkProtocol::dataTransferProgress( putWritten, putToWrite, operationInProgress() );
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
