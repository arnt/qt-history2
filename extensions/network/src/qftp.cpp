/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qftp.cpp#41 $
**
** Implementation of Network Extension Library
**
** Created : 970521
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

#include "qftp.h"
#include "qurlinfo.h"
#include <stdlib.h>
#include "qurloperator.h"

#include <qstringlist.h>
#include <qregexp.h>

QFtp::QFtp()
    : QNetworkProtocol(), connectionReady( FALSE ),
      passiveMode( FALSE )
{
    commandSocket = new QSocket( this );
    dataSocket = new QSocket( this );

    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    connect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    connect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    connect( dataSocket, SIGNAL( closed() ),
	     this, SLOT( dataClosed() ) );
    connect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );
}

QFtp::~QFtp()
{
    close();
    delete commandSocket;
    delete dataSocket;
}

void QFtp::operationListChildren( QNetworkOperation *op )
{
    commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
}

void QFtp::operationMkDir( QNetworkOperation *op )
{
    QString cmd( "MKD " + op->arg1() + "\r\n" );
    commandSocket->writeBlock( cmd, cmd.length() );
}

void QFtp::operationRemove( QNetworkOperation *op )
{
    QString path = url()->path().isEmpty() ? QString( "/" ) : url()->path();
    QString cmd = "CWD " + path + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void QFtp::operationRename( QNetworkOperation *op )
{
    QString oldname = op->arg1();
    QString newname = op->arg2();

    QString cmd( "RNFR " + oldname + "\r\n" );
    commandSocket->writeBlock( cmd, cmd.length() );
    cmd = "RNTO " + newname + "\r\n";
    commandSocket->writeBlock( cmd, cmd.length() );
}

void QFtp::operationGet( QNetworkOperation *op )
{
    commandSocket->writeBlock( "TYPE I\r\n", 8 );
}

void QFtp::operationPut( QNetworkOperation *op )
{
    commandSocket->writeBlock( "TYPE I\r\n", 8 );
}

bool QFtp::checkConnection( QNetworkOperation *op )
{
    if ( !commandSocket->host().isEmpty() && connectionReady &&
	 !passiveMode )
	return TRUE;

    if ( !commandSocket->host().isEmpty() )
	return FALSE;

    connectionReady = FALSE;
    if ( commandSocket->connectToHost( url()->host(), url()->port() != -1 ? url()->port() : 21 ) ) {
	if ( !dataSocket->host().isEmpty() )
	    dataSocket->close();
    } else {
	QString msg = tr( "Host not found: \n" + url()->host() );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrHostNotFound );
	emit finished( op );
	clearOperationQueue();
    }

    return FALSE;
}

void QFtp::close()
{
    if ( !dataSocket->host().isEmpty() ) {
	dataSocket->close();
    }
    if ( !commandSocket->host().isEmpty() ) {
 	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
 	commandSocket->close();
    }
}

int QFtp::supportedOperations() const
{
    return OpListChildren | OpMkdir | OpRemove | OpRename | OpGet | OpPut;
}

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

    info.setWritable( TRUE );
    info.setReadable( TRUE );

    // owner
    tmp = lst[ 2 ];
    info.setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info.setGroup( tmp );

    // size
    tmp = lst[ 4 ];
    info.setSize( tmp.toInt() );

    // date, time #### todo

    // name
    if ( info.isSymLink() )
	info.setName( lst[ 8 ].stripWhiteSpace() );
    else {
	QString n;
	for ( unsigned int i = 8; i < lst.count(); ++i )
	    n += lst[ i ] + " ";
	n = n.stripWhiteSpace();
	info.setName( n );
    }
}

void QFtp::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void QFtp::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
}

void QFtp::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
}

void QFtp::readyRead()
{
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );

    if ( !url() )
	return;

    bool ok = FALSE;
    int code = s.left( 3 ).toInt( &ok );
    if ( !ok )
	return;

    //qDebug( "%s", s.data() );

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
    else
	;// starnge things happen...
}

void QFtp::okButTryLater( int code, const QCString & )
{
    switch ( code ) {
    }
}

void QFtp::okGoOn( int code, const QCString &data )
{
    switch ( code ) {
    case 200: { // last command ok
	if ( operationInProgress() ) {
	    if ( operationInProgress()->operation() == OpGet ||
		 operationInProgress()->operation() == OpPut )
		commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	}
    } break;
    case 220: { // expect USERNAME
	QString user = url()->user().isEmpty() ? QString( "anonymous" ) : url()->user();
	QString cmd = "USER " + user + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
    } break;
    case 230: // succesfully logged in
	connectionReady = TRUE;
	break;
    case 227: { // open the data connection (passive mode)
	QCString s = data;
	int i = s.find( "(" );
	int i2 = s.find( ")" );
	s = s.mid( i + 1, i2 - i - 1 );
	if ( !dataSocket->host().isEmpty() )
	    dataSocket->close();
	QStringList lst = QStringList::split( ',', s );
	int port = ( lst[ 4 ].toInt() << 8 ) + lst[ 5 ].toInt();
	dataSocket->connectToHost( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3 ], port );
	if ( operationInProgress() && operationInProgress()->operation() == OpListChildren )
	    dataSocket->setMode( QSocket::Ascii );
    } break;
    case 250: { // file operation succesfully
	if ( operationInProgress() && !passiveMode &&
	     operationInProgress()->operation() == OpListChildren ) { // list dir
	    dataSocket->setMode( QSocket::Ascii );
	    commandSocket->writeBlock( "LIST\r\n", strlen( "LIST\r\n" ) );
	    emit start( operationInProgress() );
	    passiveMode = TRUE;
	} else if ( operationInProgress() &&
		    operationInProgress()->operation() == OpRename ) { // rename successfull
	    operationInProgress()->setState( StDone );
	    emit itemChanged( operationInProgress() );
	    emit finished( operationInProgress() );
	} else if ( operationInProgress() &&
		    operationInProgress()->operation() == OpRemove ) { // remove or cwd successful
	    if ( operationInProgress()->state() == StWaiting ) {
		operationInProgress()->setState( StInProgress );
		QString cmd( "DELE " + operationInProgress()->arg1() + "\r\n" );
		commandSocket->writeBlock( cmd, cmd.length() );
	    } else {
		operationInProgress()->setState( StDone );
		emit removed( operationInProgress() );
		emit finished( operationInProgress() );
	    }
	}
    } break;
    case 226: // listing directory (in passive mode) finished and data socket closing
	break;
    case 257: { // mkdir worked
	if ( operationInProgress() && operationInProgress()->operation() == OpMkdir ) {
	    operationInProgress()->setState( StDone );
	    // ######## todo get correct info
	    QUrlInfo inf( operationInProgress()->arg1(), 0, "", "", 0, QDateTime(),
			  QDateTime(), TRUE, FALSE, FALSE, TRUE, TRUE, TRUE );
	    emit newChild( inf, operationInProgress() );
	    emit createdDirectory( inf, operationInProgress() );
	    emit finished( operationInProgress() );
	    reinitCommandSocket();
	}
    } break;
    }
}

void QFtp::okButNeedMoreInfo( int code, const QCString & )
{
    switch ( code ) {
    case 331: // expect PASSWORD
	QString pass = url()->pass().isEmpty() ? QString( "info@troll.no" ) : url()->pass();
	QString cmd = "PASS " + pass + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
	break;
    }
}

void QFtp::errorForNow( int code, const QCString & )
{
    switch ( code ) {
    }
}

void QFtp::errorForgetIt( int code, const QCString &data )
{
    switch ( code ) {
    case 530: { // Login incorrect
	close();
	QString msg( tr( "Login Incorrect" ) );
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    qDebug( "%d", op->operation() );
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( ErrLoginIncorrect );
	}
	clearOperationQueue();
	emit finished( op );
	reinitCommandSocket();
    } break;
    case 550: { // no such file or directory
	QString msg( data.mid( 4 ) );
	msg = msg.simplifyWhiteSpace();
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( ErrFileNotExisting );
	}
	emit finished( op );
	reinitCommandSocket();
    } break;
    case 553: { // permission denied
	QString msg( data.mid( 4 ) );
	msg = msg.simplifyWhiteSpace();
	QNetworkOperation *op = operationInProgress();
	if ( op ) {
	    op->setProtocolDetail( msg );
	    op->setState( StFailed );
	    op->setErrorCode( ErrPermissionDenied );
	}
	emit finished( op );
	reinitCommandSocket();
    } break;
    }
}

void QFtp::dataHostFound()
{
}

void QFtp::dataConnected()
{
    if ( !operationInProgress() )
	return;
    switch ( operationInProgress()->operation() ) {
    case OpListChildren: { // change dir first
	QString path = url()->path().isEmpty() ? QString( "/" ) : url()->path();
	QString cmd = "CWD " + path + "\r\n";
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    } break;
    case OpGet: { // retrieve file
	if ( !operationInProgress() || operationInProgress()->arg1().isEmpty() ) {
	    qWarning( "no filename" );
	    break;
	}
	QString cmd = "RETR " + QUrl( operationInProgress()->arg1() ).path() + "\r\n";
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    } break;
    case OpPut: { // upload file
	if ( !operationInProgress() || operationInProgress()->arg1().isEmpty() ) {
	    qWarning( "no filename" );
	    break;
	}
	QString cmd = "STOR " + QUrl( operationInProgress()->arg1() ).path() + "\r\n";
	commandSocket->writeBlock( cmd.latin1(), cmd.length() );
	dataSocket->writeBlock( operationInProgress()->rawArg2(),
				operationInProgress()->rawArg2().size() );
	dataSocket->close();
    } break;
    }
}

void QFtp::dataClosed()
{
    emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
    // switch back to ASCII mode
    commandSocket->writeBlock( "TYPE A\r\n", 8 );

    passiveMode = FALSE;
    emit finished( operationInProgress() );

    disconnect( dataSocket, SIGNAL( hostFound() ),
		this, SLOT( dataHostFound() ) );
    disconnect( dataSocket, SIGNAL( connected() ),
		this, SLOT( dataConnected() ) );
    disconnect( dataSocket, SIGNAL( closed() ),
		this, SLOT( dataClosed() ) );
    disconnect( dataSocket, SIGNAL( readyRead() ),
		this, SLOT( dataReadyRead() ) );
    delete dataSocket;
    dataSocket = new QSocket( this );
    connect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    connect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    connect( dataSocket, SIGNAL( closed() ),
	     this, SLOT( dataClosed() ) );
    connect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );
    reinitCommandSocket();
}

void QFtp::dataReadyRead()
{
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
		    if ( inf.isDir() || filt.match( inf.name() ) != -1 ) {
			emit newChild( inf, operationInProgress() );
		    }
		}
	    }
	}
    } break;
    case OpGet: {
	QByteArray s;
	s.resize( dataSocket->bytesAvailable() );
	dataSocket->readBlock( s.data(), dataSocket->bytesAvailable() );
	emit data( s, operationInProgress() );
	//qDebug( "%s", s.data() );
    } break;
    }
}

void QFtp::reinitCommandSocket()
{
    commandSocket->close();
    disconnect( commandSocket, SIGNAL( hostFound() ),
		this, SLOT( hostFound() ) );
    disconnect( commandSocket, SIGNAL( connected() ),
		this, SLOT( connected() ) );
    disconnect( commandSocket, SIGNAL( closed() ),
		this, SLOT( closed() ) );
    disconnect( commandSocket, SIGNAL( readyRead() ),
		this, SLOT( readyRead() ) );
    delete commandSocket;
    commandSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
}
