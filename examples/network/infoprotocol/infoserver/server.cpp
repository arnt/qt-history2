/****************************************************************************
** $Id: $
**
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qtextview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <stdlib.h>

#include "server.h"


// we hard code all nodes and data in InfoData class
InfoData::InfoData() :
    nodes( 17, TRUE ), data( 17, TRUE )
{
    nodes.setAutoDelete(TRUE);
    data.setAutoDelete(TRUE);
    QStringList *item;

    nodes.insert( "/", item = new QStringList( ) );
    (*item) << "D network";
    nodes.insert( "/network", item = new QStringList() );
    (*item) << "D workstations" << "D printers" << "D fax";
    nodes.insert( "/network/workstations", item = new QStringList() );
    (*item) << "D nibble" << "D douglas";
    nodes.insert( "/network/workstations/nibble", item = new QStringList() );
    (*item) << "F os" << "F cpu" << "F memory";
    nodes.insert( "/network/workstations/douglas", item = new QStringList() );
    (*item) << "F os" << "F cpu" << "F memory";
    nodes.insert( "/network/printers", item = new QStringList() );
    (*item) << "D overbitt" << "D kroksleiven";
    nodes.insert( "/network/printers/overbitt", item = new QStringList() );
    (*item) << "D jobs" << "F type";
    nodes.insert( "/network/printers/overbitt/jobs", item = new QStringList() );
    (*item) << "F job1" << "F job2";
    nodes.insert( "/network/printers/kroksleiven", item = new QStringList() );
    (*item) << "D jobs" << "F type";
    nodes.insert( "/network/printers/kroksleiven/jobs", item = new QStringList() );
    nodes.insert( "/network/fax", item = new QStringList() );
    (*item) << "F last_number";

    data.insert( "/network/workstations/nibble/os", new QStringList( "Linux" ) );
    data.insert( "/network/workstations/nibble/cpu", new QStringList( "AMD Athlon 1000" ) );
    data.insert( "/network/workstations/nibble/memory", new QStringList( "256 MB" ) );
    data.insert( "/network/workstations/douglas/os", new QStringList( "Windows 2000" ) );
    data.insert( "/network/workstations/douglas/cpu", new QStringList( "2 x Intel Pentium III 800" ) );
    data.insert( "/network/workstations/douglas/memory", new QStringList( "256 MB" ) );
    data.insert( "/network/printers/overbitt/type", new QStringList( "Lexmark Optra S 1255 PS") );
    data.insert( "/network/printers/overbitt/jobs/job1", item = new QStringList( "Qt manual") );
    (*item) << "A4 size" << "3000 pages";
    data.insert( "/network/printers/overbitt/jobs/job2", item = new QStringList( "Monthly report") );
    (*item) << "Letter size" << "24 pages" << "8 copies";
    data.insert( "/network/printers/kroksleiven/type", new QStringList( "HP C LaserJet 4500-PS") );
    data.insert( "/network/fax/last_number", new QStringList( "22 22 22 22") );
}

QStringList* InfoData::list( const QString& path ) const
{
    return nodes[ path ];
}

QStringList* InfoData::get( const QString& path ) const
{
    return data[ path ];
}



ClientSocket::ClientSocket( int sock, InfoData *i, QObject *parent, const char *name ) :
    QSocket( parent, name ), info( i )
{
    connect( this, SIGNAL(readyRead()), SLOT(readClient()) );
    connect( this, SIGNAL(connectionClosed()), SLOT(connectionClosed()) );
    setSocket( sock );
}

void ClientSocket::readClient()
{
    while ( canReadLine() ) {
	QTextStream stream( this );
	QStringList answer;
	if ( processCommand( stream.readLine(), &answer ) ) 
	    for ( QStringList::Iterator it = answer.begin(); it != answer.end(); ++it ) {
		stream << *it;
	    }
    }
}

void ClientSocket::connectionClosed()
{
    delete this;
}

bool ClientSocket::processCommand( const QString& command, QStringList *answer )
{
    answer->clear();
    QString com = command.simplifyWhiteSpace ();
    if ( command.isNull() || command.isEmpty() )
	return FALSE;
    if ( com.startsWith( "LIST" ) ) {
	com = com.mid( 5 );
	if ( com.endsWith( "/" ) )
	    com.truncate( com.length() - 1 );
	if ( !com.startsWith( "/" ) )
	    com = "/" + com;
	QStringList* nodes = info->list( com );
	if ( nodes ) {
	    for ( uint i = 0; i < nodes->count(); ++i ) {
		answer->append( "212+" + (*nodes)[i] + "\r\n" );
	    }
	    answer->append( "212 \r\n" );
	} else {
	    answer->append( "550 File not found\r\n" );	
	}
    } else if ( com.startsWith( "GET " ) ) {
	com = com.mid( 4 );
	if ( !com.startsWith( "/" ) )
	    com = "/" + com;
	QStringList* data = info->get( com );
	if ( data ) {
	    for ( uint i = 0; i < data->count(); ++i ) {
		answer->append( "213+" + (*data)[i] + "\r\n" );
	    }
	    answer->append( "213 \r\n" );
	} else {
	    answer->append( "550 File not found\r\n" );	
	}
    } else {
	answer->append( "500 Syntax error\r\n" );	
    }
    return TRUE;	
}



SimpleServer::SimpleServer( InfoData *i, Q_UINT16 port, QObject* parent ) :
    QServerSocket( port, 1, parent ), info( i )
{
    if ( !ok() ) {
	QMessageBox::critical( 0, tr( "Error" ), tr( "Failed to bind to port %1" ).arg( port ) );
	exit(1);
    }
}

void SimpleServer::newConnection( int socket )
{
    (void)new ClientSocket( socket, info, this );
    emit newConnect();
}


ServerInfo::ServerInfo()
{
    Q_UINT16 port = ( qApp->argc() > 1 ) ? QString( qApp->argv()[ 1 ] ).toInt() : infoPort;
    SimpleServer *server = new SimpleServer( &info, port, this );
    connect( server, SIGNAL(newConnect()), SLOT(newConnect()) );
    connect( btnQuit, SIGNAL(clicked()), qApp, SLOT(quit()) );
}

void ServerInfo::newConnect()
{
    infoText->append( tr( "New connection\n" ) );
}

