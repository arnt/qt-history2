/****************************************************************************
** $Id$
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "http.h"

#ifndef QT_NO_NETWORKPROTOCOL_HTTP

#include "qurlinfo.h"
#include <stdlib.h>

#include <qstringlist.h>
#include <qregexp.h>

Http::Http()
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
}

Http::~Http()
{
    close();
    delete commandSocket;
}

void Http::operationPut( QNetworkOperation * )
{
    QString cmd = "POST ";
    cmd += url()->encodedPathAndQuery();
    cmd += "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void Http::operationGet( QNetworkOperation * )
{
    QString cmd = "GET ";
    cmd += url()->encodedPathAndQuery();
    cmd += "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

bool Http::checkConnection( QNetworkOperation * )
{
    if ( !commandSocket->peerName().isEmpty() && connectionReady )
	return TRUE;

    if ( !commandSocket->peerName().isEmpty() )
	return FALSE;

    connectionReady = FALSE;
    commandSocket->connectToHost( url()->host(),
				  url()->port() != -1 ? url()->port() : 80 );
#if 0
    // connectToHost just starts something - there's no point in
    // making any tests until something has responded.
    if ( !commandSocket->connectToHost( url()->host(), url()->port() != -1 ? url()->port() : 80 ) ) {
	QString msg = tr( "Host not found: \n" + url()->host() );
	op->setState( (int)StFailed );
	op->setProtocolDetail( msg );
	op->setErrorCode( ErrHostNotFound );
    }
#endif

    return FALSE;
}

void Http::close()
{
    if ( !commandSocket->peerName().isEmpty() ) {
 	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
 	commandSocket->close();
    }
}

int Http::supportedOperations() const
{
    return OpGet | OpPut;
}

void Http::hostFound()
{
    if ( url() )
	emit connectionStateChanged( ConHostFound, tr( "Host %1 found" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConHostFound, tr( "Host found" ) );
}

void Http::connected()
{
    if ( url() )
	emit connectionStateChanged( ConConnected, tr( "Connected to host %1" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConConnected, tr( "Connected to host" ) );
    connectionReady = TRUE;
}

void Http::closed()
{
    if ( url() )
	emit connectionStateChanged( ConClosed, tr( "Connection to %1 closed" ).arg( url()->host() ) );
    else
	emit connectionStateChanged( ConClosed, tr( "Connection closed" ) );

    connectionReady = FALSE;
    emit finished( operationInProgress() );

}

void Http::readyRead()
{
    QByteArray s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );
    emit data( s, operationInProgress() );
}

#endif // QT_NO_NETWORKPROTOCOL_HTTP
