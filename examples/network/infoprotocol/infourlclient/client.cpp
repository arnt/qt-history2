/****************************************************************************
** $Id: $
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qsocket.h>
#include <qapplication.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qurlinfo.h>
#include <qfiledialog.h>

#include "client.h"


Qip::Qip()
{
    state = Start;
    socket = new QSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(socketConnected()) );
    connect( socket, SIGNAL(connectionClosed()), SLOT(socketConnectionClosed()) );
    connect( socket, SIGNAL(readyRead()), SLOT(socketReadyRead()) );
    connect( socket, SIGNAL(error(int)), SLOT(socketError(int)) );
}

int Qip::supportedOperations() const
{
    // we only support listing children and getting data
    return OpListChildren | OpGet;
}

bool Qip::checkConnection( QNetworkOperation * )
{
    // we are connected, return TRUE
    if ( socket->isOpen() )
	return TRUE;

    // don't call connectToHost() if we are already trying to connect
    if ( socket->state() == QSocket::Connecting )
	return FALSE;

    // start connecting
    socket->connectToHost( url()->host(), url()->port() != -1 ? url()->port() : infoPort );
    return FALSE;
}

void Qip::operationListChildren( QNetworkOperation * )
{
    // send a LIST command
    QTextStream os(socket);
    os << "LIST " + url()->path() + "\r\n";
    operationInProgress()->setState( StInProgress );
}

void Qip::operationGet( QNetworkOperation * )
{
    // send a GET command
    QTextStream os(socket);
    os << "GET " + url()->path() + "\r\n";
    operationInProgress()->setState( StInProgress );
}

void Qip::socketConnected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, QString( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, "Connected to host" );
}

void Qip::socketConnectionClosed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, QString( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, "Connection closed" );
}

void Qip::socketError( int code )
{
    if ( operationInProgress() ) {
	QString msg( "Error" );
	int errCode( ErrUnsupported );
	if ( code == QSocket::ErrHostNotFound ||
	     code == QSocket::ErrConnectionRefused ) {
	    // this signal is called if connecting to the server failed
	    msg = "Host not found or couldn't connect to: \n" + url()->host();
	    errCode = ErrHostNotFound;
	} else if ( code == ErrFileNotExisting ) {
	    msg = "File not found: \n" + url()->path();
	    errCode = ErrFileNotExisting;
	} else if ( code == ErrValid ) {
	    msg = "Syntax error";
	    errCode = ErrValid;
	}
	operationInProgress()->setState( StFailed );
	operationInProgress()->setErrorCode( errCode );
	operationInProgress()->setProtocolDetail( msg );
	clearOperationQueue();
	emit finished( operationInProgress() );
    }
    state = Start;
}

void Qip::socketReadyRead()
{
    // read from the server
    QTextStream stream( socket );
    QString line;
    while ( socket->canReadLine() ) {
	line = stream.readLine();
	if ( line.startsWith( "500" ) ) {
	    socketError( ErrValid ); 
	} else if ( line.startsWith( "550" ) ) {
	    socketError( ErrFileNotExisting ); 
	} else if ( line.startsWith( "212+" ) ) {
	    if ( state != List ) {
		state = List;
	        emit start( operationInProgress() );
	    }
	    QUrlInfo inf;
	    inf.setName( line.mid( 6 ) + QString( ( line[ 4 ] == 'D' ) ? "/" : "" ) );
	    inf.setDir( line[ 4 ] == 'D' );
	    inf.setSymLink( FALSE );
	    inf.setFile( line[ 4 ] == 'F' );
	    inf.setWritable( FALSE );
	    inf.setReadable( TRUE );
	    emit newChild( inf, operationInProgress() );
	} else if ( line.startsWith( "213+" ) ) {
	    state = Data;
	    emit data( QCString( line.mid( 4 ).latin1() ), operationInProgress() );
	}
	if( line[3] == ' ' && state != Start) {
	    state = Start;
	    operationInProgress()->setState( StDone );
	    emit finished( operationInProgress() );
	}
    }
}



ClientInfo::ClientInfo()
{
    QNetworkProtocol::registerNetworkProtocol( "qip", new QNetworkProtocolFactory< Qip > );
    connect( btnOpen, SIGNAL(clicked()), SLOT(downloadFile()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
    // if new data comes in, display it
    connect( &op, SIGNAL( data( const QByteArray &, QNetworkOperation * ) ),
	     this, SLOT( newData( const QByteArray & ) ) );
}

void ClientInfo::downloadFile()
{
    // QString file = QFileDialog::getOpenFileName();
    // under Windows you must not use the native file dialog
    QString file = getOpenFileName();
    if ( !file.isEmpty() ) {
	// clear the view
	infoText->clear();
	// download the data
	op = file;
	op.get();
    }
}

QString ClientInfo::getOpenFileName()
{
    static QString workingDirectory( "qip://localhost/" );

    QFileDialog *dlg = new QFileDialog( workingDirectory,
	    QString::null, 0, 0, TRUE );
    dlg->setCaption( "Open" );
    dlg->setMode( QFileDialog::ExistingFile );
    QString result;
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	workingDirectory = dlg->url();
    }
    delete dlg;
    return result;
}

void ClientInfo::newData( const QByteArray &ba )
{
    // append new data
    infoText->append( ba );
}
