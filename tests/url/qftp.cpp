/****************************************************************************
** $Id: //depot/qt/main/tests/url/qftp.cpp#2 $
**
** Implementation of QFileDialog class
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

#include "qftp.h"
#include "qurlinfo.h"
#include <stdlib.h>

#include <qstringlist.h>
#include <qregexp.h>

void QFtp::hostFound()
{
}

void QFtp::connected()
{
}

void QFtp::closed()
{
}

void QFtp::dataHostFound()
{
}

void QFtp::dataConnected()
{
    qDebug( "data host connected" );
    QString cmd = "CWD " + path + "\r\n";
    commandSocket->writeBlock( cmd.latin1(), cmd.length() );
}

void QFtp::dataClosed()
{
    emit listFinished();
    qDebug( "CLOOOOSED" );
}

void QFtp::dataReadyRead()
{
    qDebug( "dataReadyRead" );
    QCString s;
    s.resize( dataSocket->bytesAvailable() );
    dataSocket->readBlock( s.data(), dataSocket->bytesAvailable() );
    QString ss = s.copy();
    QStringList lst = QStringList::split( '\n', ss );
    QStringList::Iterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
	QUrlInfo inf;
	parseDir( *it, inf );
	if ( !inf.name().isEmpty() )
	    emit newEntry( inf );
    }
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

void QFtp::readyRead()
{
    QCString s;
    s.resize( commandSocket->bytesAvailable() );
    commandSocket->readBlock( s.data(), commandSocket->bytesAvailable() );
	
    if ( s.contains( "220" ) ) {
	QString cmd = "USER " + username + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
    } else if ( s.contains( "331" ) ) {
	QString cmd = "PASS " + passwd + "\r\n";
	commandSocket->writeBlock( cmd, cmd.length() );
    } else if ( s.contains( "230" ) ) {
	switch ( command ) {
	case List:
	    commandSocket->writeBlock( "PASV\r\n", strlen( "PASV\r\n") );
	    break;
	case Mkdir:
	    QString cmd( "MKD " + extraData + "\r\n" );
	    qDebug( "mkdir: %s", extraData.latin1() );
	    commandSocket->writeBlock( cmd, cmd.length() );
	    break;
	}
    } else if ( s.contains( "227" ) ) {
	int i = s.find( "(" );
	int i2 = s.find( ")" );
	s = s.mid( i + 1, i2 - i - 1 );
	if ( !dataSocket->host().isEmpty() )
	    return;//dataSocket->close();
	QStringList lst = QStringList::split( ',', s );
	int port = ( lst[ 4 ].toInt() << 8 ) + lst[ 5 ].toInt();
	qDebug( "url: %s port: %d", QString( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3] ).latin1(),
		port );
	dataSocket->connectToHost( lst[ 0 ] + "." + lst[ 1 ] + "." + lst[ 2 ] + "." + lst[ 3 ], port );
    } else if ( s.contains( "250" ) ) {
	qDebug( "cwd successfully" );
	commandSocket->writeBlock( "LIST\r\n", strlen( "LIST\r\n" ) );
    } else
	qDebug( "unknown result: %s", s.data() );
	
}

QFtp::QFtp()
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

void QFtp::open( const QString &host_, int port, const QString &path_,
		const QString &username_, const QString &passwd_,
		Command cmd, const QString &extraData_ )

{
    commandSocket->connectToHost( host_, port );
    if ( !dataSocket->host().isEmpty() )
	dataSocket->close();
    host = host_;
    path = path_;
    username = username_;
    passwd = passwd_;
    command = cmd;
    extraData = extraData_;
}

void QFtp::close()
{	
    if ( !commandSocket->host().isEmpty() ) {
	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
	commandSocket->close();
    }
}

QFtp::~QFtp()
{
    if ( !commandSocket->host().isEmpty() ) {
	commandSocket->writeBlock( "quit\r\n", strlen( "quit\r\n" ) );
	commandSocket->close();
    }
    delete commandSocket;
    commandSocket = 0;
}

QFtp &QFtp::operator=( const QFtp &ftp )
{
    disconnect( commandSocket, SIGNAL( hostFound() ),
		this, SLOT( hostFound() ) );
    disconnect( commandSocket, SIGNAL( connected() ),
		this, SLOT( connected() ) );
    disconnect( commandSocket, SIGNAL( closed() ),
		this, SLOT( closed() ) );
    disconnect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );
    commandSocket = new QSocket( this );
    connect( commandSocket, SIGNAL( hostFound() ),
	     this, SLOT( hostFound() ) );
    connect( commandSocket, SIGNAL( connected() ),
	     this, SLOT( connected() ) );
    connect( commandSocket, SIGNAL( closed() ),
	     this, SLOT( closed() ) );
    connect( commandSocket, SIGNAL( readyRead() ),
	     this, SLOT( readyRead() ) );

    disconnect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    disconnect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    disconnect( dataSocket, SIGNAL( closed() ),
	     this, SLOT( dataClosed() ) );
    disconnect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );
    dataSocket = new QSocket( this );
    connect( dataSocket, SIGNAL( hostFound() ),
	     this, SLOT( dataHostFound() ) );
    connect( dataSocket, SIGNAL( connected() ),
	     this, SLOT( dataConnected() ) );
    connect( dataSocket, SIGNAL( closed() ),
	     this, SLOT( dataClosed() ) );
    connect( dataSocket, SIGNAL( readyRead() ),
	     this, SLOT( dataReadyRead() ) );


    if ( !ftp.commandSocket->host().isEmpty() )
	commandSocket->connectToHost( ftp.commandSocket->host(),
				      ftp.commandSocket->port() );
    if ( !ftp.dataSocket->host().isEmpty() )
	dataSocket->connectToHost( ftp.dataSocket->host(),
				   ftp.dataSocket->port() );
    host = ftp.host;
    path = ftp.path;
    buffer = ftp.buffer;

    return *this;
}
