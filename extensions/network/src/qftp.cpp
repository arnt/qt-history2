/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qftp.cpp#1 $
**
** Implementation of JPEG QImage IOHandler
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

#include <qstringlist.h>
#include <qregexp.h>

QFtp::QFtp()
    : QNetworkProtocol(), connectionReady( FALSE )
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

void QFtp::openConnection( QUrl *u )
{
    QNetworkProtocol::openConnection( u );
    connectionReady = FALSE;
    commandSocket->connectToHost( url->host(), 21 /*url->port()*/ ); //####
    if ( !dataSocket->host().isEmpty() )
	dataSocket->close();
    extraData = QString::null;
}

bool QFtp::isOpen()
{
    return !commandSocket->host().isEmpty();
}

void QFtp::close()
{
    if ( !dataSocket->host().isEmpty() )
	dataSocket->close();
    if ( !commandSocket->host().isEmpty() ) {
 	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
 	commandSocket->close();
    }
}

void QFtp::listEntries( const QString &/*nameFilter*/, int /*filterSpec*/, int /*sortSpec*/ )
{
    if ( !isOpen() ) {
	if ( url )
	    openConnection( url );
	else {
	    qWarning( "Cannnot open FTP connection, URL is NULL!" );
	    return;
	}
    }

    command = List;
    if ( connectionReady )
	commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
}

void QFtp::mkdir( const QString &dirname )
{
    if ( !isOpen() ) {
	if ( url )
	    openConnection( url );
	else {
	    qWarning( "Cannnot open FTP connection, URL is NULL!" );
	    return;
	}
    }

    command = Mkdir;
    extraData = dirname;
    if ( connectionReady ) {
	QString cmd( "MKD " + extraData + "\r\n" );
	commandSocket->writeBlock( cmd, cmd.length() );
    }
}

void QFtp::remove( const QString &/*filename*/ )
{
}

void QFtp::rename( const QString &/*oldname*/, const QString &/*newname*/ )
{
}

void QFtp::copy( const QStringList &/*files*/, const QString &/*dest*/, bool /*move*/ )
{
}

QNetworkProtocol *QFtp::copy() const
{
    return new QFtp;
}

void QFtp::isDir()
{
    // #### todo
    if ( url )
	url->emitUrlIsDir();
}

void QFtp::isFile()
{
    // #### todo
}

QString QFtp::toString() const
{
    if ( !url->user().isEmpty() )
	return url->protocol() + "://" + url->user() + ":" + url->pass() + "@" + url->host() +
	    QDir::cleanDirPath( url->path() ).stripWhiteSpace(); // #### todo
    else
	return url->protocol() + "://" + url->host() + QDir::cleanDirPath( url->path() ).stripWhiteSpace(); // #### todo

    return QString::null;
}

void QFtp::parseDir( const QString &buffer, QUrlInfo &info )
{
    QStringList lst = QStringList::split( " ", buffer );
    QString tmp;

    // permissions
    tmp = lst[ 0 ];

    if ( tmp[ 0 ] == QChar( 'd' ) ) {
	info.setDir( TRUE );
	info.setFile( FALSE );
    } else if ( tmp[ 0 ] == QChar( '-' ) ) {
	info.setDir( FALSE );
	info.setFile( TRUE );
    } else
	return; // ### todo links

    // owner
    tmp = lst[ 2 ];
    info.setOwner( tmp );

    // group
    tmp = lst[ 3 ];
    info.setGroup( tmp );

    // date, time #### todo

    // name
    info.setName( lst[ 8 ].stripWhiteSpace() );
}

void QFtp::hostFound()
{
}

void QFtp::connected()
{
}

void QFtp::closed()
{
}

void QFtp::readyRead()
{
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );
	
    if ( !url )
	return;

    if ( s.contains( "220" ) ) { // expect USERNAME
	QString user = url->user().isEmpty() ? QString( "anonymous" ) : url->user();
	QString cmd = "USER " + user + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
    } else if ( s.contains( "331" ) ) { // expect PASSWORD
	QString pass = url->pass().isEmpty() ? QString( "info@troll.no" ) : url->pass();
	QString cmd = "PASS " + pass + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
	connectionReady = FALSE;
    } else if ( s.contains( "230" ) ) { // succesfully logged in
	connectionReady = TRUE;
	switch ( command ) {
	case List:
	    commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	    break;
	case Mkdir: {
	    QString cmd( "MKD " + extraData + "\r\n" );
	    commandSocket->writeBlock( cmd, cmd.length() );
	    } break;
	case None:
	    break;
	}
    } else if ( s.contains( "227" ) ) { // open the data connection for LIST
	int i = s.find( "(" );
	int i2 = s.find( ")" );
	s = s.mid( i + 1, i2 - i - 1 );
	if ( !dataSocket->host().isEmpty() )
	    dataSocket->close();
	QStringList lst = QStringList::split( ',', s );
	int port = ( lst[ 4 ].toInt() << 8 ) + lst[ 5 ].toInt();
	dataSocket->connectToHost( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3 ], port );
    } else if ( s.contains( "250" ) ) { // cwd succesfully, list dir
	commandSocket->writeBlock( "LIST\r\n", strlen( "LIST\r\n" ) );
    } else
	qWarning( "unknown result: %s", s.data() );
}

void QFtp::dataHostFound()
{
}

void QFtp::dataConnected()
{
    QString path = url->path().isEmpty() ? QString( "/" ) : url->path();
    QString cmd = "CWD " + path + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void QFtp::dataClosed()
{
    if ( url )
	url->emitFinished();
}

void QFtp::dataReadyRead()
{
    QCString s;
    s.resize( dataSocket->bytesAvailable() );
    dataSocket->readBlock( s.data(), dataSocket->bytesAvailable() );
    QString ss = s.copy();
    QStringList lst = QStringList::split( '\n', ss );
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
	QUrlInfo inf;
	parseDir( *it, inf );
	if ( !inf.name().isEmpty() ) {
	    if ( url )
		url->emitEntry( inf );
	}
    }
}
