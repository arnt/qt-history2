/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qhttp.cpp#1 $
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

#include "qhttp.h"
#include "qurlinfo.h"
#include <stdlib.h>

#include <qstringlist.h>
#include <qregexp.h>

QHttp::QHttp()
    : QNetworkProtocol(), connectionReady( FALSE )
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

    connect( &pixmapLoader, SIGNAL( data( const QCString &, QNetworkOperation * ) ),
	     this, SLOT( newPixmap( const QCString &, QNetworkOperation * ) ) );
    connect( &pixmapLoader, SIGNAL( finished( QNetworkOperation * ) ),
	     this, SLOT( savePixmap( QNetworkOperation * ) ) );
}

QHttp::~QHttp()
{
    close();
    delete commandSocket;
}

void QHttp::operationPost( QNetworkOperation *op )
{
    pixNum = 0;
    imgMap.clear();
    QString cmd = "POST ";
    cmd += url()->encodedPathAndQuery();
    cmd += "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void QHttp::operationGet( QNetworkOperation *op )
{
    pixNum = 0;
    imgMap.clear();
    QString cmd = "GET ";
    cmd += url()->encodedPathAndQuery();
    cmd += "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

bool QHttp::checkConnection( QNetworkOperation *op )
{
    if ( !commandSocket->host().isEmpty() && connectionReady )
	return TRUE;

    if ( !commandSocket->host().isEmpty() )
	return FALSE;

    connectionReady = FALSE;
    if ( !commandSocket->connectToHost( url()->host(), url()->port() != -1 ? url()->port() : 80 ) ) {
	QString msg = tr( "Host not found: \n" + url()->host() );
	op->setState( StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrHostNotFound );
    }

    return FALSE;
}

void QHttp::close()
{
    if ( !commandSocket->host().isEmpty() ) {
 	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
 	commandSocket->close();
    }
}

int QHttp::supportedOperations() const
{
    return OpGet | OpPost;
}

void QHttp::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void QHttp::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
    connectionReady = TRUE;
}

void QHttp::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );

    connectionReady = FALSE;
    if ( !imgMap.isEmpty() ) {
	imgIt = imgMap.begin();
	currPix = imgIt.key();
	pixmapLoader = QUrlOperator( *url(), *imgIt );
	pixmapLoader.get( "" );
    } else
	emit finished( operationInProgress() );

}

void QHttp::readyRead()
{
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );

    int i = 0;
    i = s.find( "<img", 0 );
    while ( i != -1 ) {
	int st = 0, e = 0;
	st = s.find( "src", i ) + 3;
	while ( s[ st ] == '=' || s[ st ] == ' ' || s[ st ] == '"' )
	    ++st;
	e = st;
	while ( s[ e ] != ' ' && s[ st ] != '"' )
	    ++e;
	QString img = s.mid( st, e - st - 1 );

	QString imgname( "/tmp/pic_%1.%2" );
	imgname = imgname.arg( ++pixNum ).arg( QFileInfo( img ).extension() );
	
	imgMap[ imgname ] = img;
	s = s.replace( st, img.length(), imgname );
	
	i = s.find( "<img", e );
    }
    
    emit data( s, operationInProgress() );
    buffer += s;
}

void QHttp::newPixmap( const QCString &data, QNetworkOperation * )
{
    pixBuff += data;
}

void QHttp::savePixmap( QNetworkOperation * )
{
    QFile f( currPix );
    f.open( IO_WriteOnly );
    f.writeBlock( pixBuff, pixBuff.length() );
    f.close();
    
    pixBuff = "";
    
    ++imgIt;
    if ( imgIt == imgMap.end() ) {
	emit data( '\0', operationInProgress() );
	emit finished( operationInProgress() );
	return;
    }

    currPix = imgIt.key();
    pixmapLoader = QUrlOperator( *url(), *imgIt );
    pixmapLoader.get( "" );

}

