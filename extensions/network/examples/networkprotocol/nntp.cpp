/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "nntp.h"
#include <qurlinfo.h>
#include <stdlib.h>
#include <qurloperator.h>
#include <qstringlist.h>
#include <qregexp.h>

Nntp::Nntp()
    : QNetworkProtocol(), connectionReady( FALSE ),
      readGroups( FALSE ), readHead( FALSE ), readBody( FALSE )
{
    commandSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    commandSocket->setMode( QSocket::Ascii );
}

Nntp::~Nntp()
{
    close();
    delete commandSocket;
}

void Nntp::operationListChildren( QNetworkOperation * )
{
    commandSocket->setMode( QSocket::Ascii );

    QString path = url()->path(), cmd;
    if ( path.isEmpty() || path == "/" ) {
	cmd = "list newsgroups\r\n";
    } else if ( url()->isDir() ) {
	path = path.replace( QRegExp( "/" ), "" );
	cmd = "listgroup " + path + "\r\n";
    } else
	return;

    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    readGroups = TRUE;
}

void Nntp::operationGet( QNetworkOperation *op )
{
    commandSocket->setMode( QSocket::Ascii );
    QUrl u( op->arg1() );
    QString dirPath = u.dirPath(), file = u.fileName();
    dirPath = dirPath.replace( QRegExp( "/" ), "" );
    QString cmd;
    cmd = "group " + dirPath + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    cmd = "head " + file + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
    readHead = TRUE;
}

bool Nntp::checkConnection( QNetworkOperation * )
{
    if ( !commandSocket->host().isEmpty() && connectionReady )
	return TRUE;

    if ( !commandSocket->host().isEmpty() )
	return FALSE;

    connectionReady = FALSE;
    commandSocket->connectToHost( url()->host(),
				  url()->port() != -1 ? url()->port() : 119 );
    return FALSE;
}

void Nntp::close()
{
    if ( !commandSocket->host().isEmpty() ) {
 	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
 	commandSocket->close();
    }
}

int Nntp::supportedOperations() const
{
    return OpListChildren | OpGet;
}

void Nntp::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void Nntp::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
}

void Nntp::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );
}

void Nntp::readyRead()
{
    if ( readGroups ) {
	parseGroups();
	return;
    }

    if ( readHead || readBody ) {
	readArticle();
	return;
    }
    
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );

    if ( !url() )
	return;

    if ( s.left( 3 ) == "200" )
	connectionReady = TRUE;
}

void Nntp::parseGroups()
{
    if ( !commandSocket->canReadLine() )
	return;
    while ( commandSocket->canReadLine() ) {
	QString s = commandSocket->readLine();
	if ( s[ 0 ] == '.' ) {
	    readGroups = FALSE;
	    operationInProgress()->setState( StDone );
	    emit finished( operationInProgress() );
	    return;
	}
	
	if ( s.left( 3 ) == "215" ) {
	    operationInProgress()->setState( StInProgress );
	    emit start( operationInProgress() );
	    continue;
	}
	
	bool tab = s.find( '\t' ) != -1;
	QString group = s.mid( 0, s.find( tab ? '\t' : ' ' ) );
	QUrlInfo inf;
	inf.setName( group );
	QString path = url()->path();
	inf.setDir( path.isEmpty() || path == "/" );
	inf.setSymLink( FALSE );
	inf.setFile( !inf.isDir() );
	inf.setWritable( FALSE );
	inf.setReadable( TRUE );
	emit newChild( inf, operationInProgress() );
    }
	
}

void Nntp::readArticle()
{
    if ( !commandSocket->canReadLine() )
	return;
    while ( commandSocket->canReadLine() ) {
	QString s = commandSocket->readLine();
	if ( s[ 0 ] == '.' ) {
	    if ( readHead ) {
		readHead = FALSE;
		readBody =TRUE;
		QString cmd = "body " + QUrl( operationInProgress()->arg1() ).fileName() + "\r\n";
		commandSocket->writeBlock( cmd.latin1(), cmd.length() );
	    } else {
		readBody = FALSE;
		operationInProgress()->setState( StDone );
		emit finished( operationInProgress() );
	    }
	    return;
	}
    
	if ( s.right( 1 ) == "\n" )
	    s.remove( s.length() - 1, 1 );
	emit data( QCString( s.ascii() ), operationInProgress() );
    }
}
