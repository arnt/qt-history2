/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "smtp.h"

#include <qsocket.h>
#include <qtimer.h>
#include <qapplication.h>

/*
  As the Qt Network Architecture works on hirarchical structures, it's
  not useful for each implementation of a network protocol. Such an example
  is sending mails via SMTP. As this is one of the simplest protocols, and has
  nothing todo with hirachical structures, you just need a socket and write some 
  stuff to it. Here is an implementation of the SMTP protocol using QSocket and 
  nothing else of the Qt network Architecture is used.
*/

Smtp::Smtp( const QString &server_, int port_,
	    const QString &from_, const QString &to_, const QString &subject_,
	    const QString &cc_, const QString &bcc_, const QString &message_ )
    : server( server_ ), from( from_ ), to( to_ ), subject( subject_ ),
      cc( cc_ ), bcc( bcc_ ), message( message_ ), port( port_ )
{
    // create socket and connect to some signals
    socket = new QSocket( this );
    connect ( socket, SIGNAL( readyRead() ),
	      this, SLOT( readyRead() ) );
    connect ( socket, SIGNAL( connected() ),
	      this, SLOT( connected() ) );

    // connect to SMTP server
    socket->connectToHost( server, port );

    // To know what we have to write at which time, the
    // protocol is implemented as state machine. Set the state
    // to Init at the beginning.
    state = Init;
}

void Smtp::connected()
{
    // Connection is up - send greetings to the server
    QString cmd = "HELO " + server + "\r\n";
    socket->writeBlock( cmd, cmd.length() );
    
    // Next state
    state = Mail;
}

void Smtp::readyRead()
{
    QCString s;
    s.resize( socket->bytesAvailable() );
    socket->readBlock( s.data(), socket->bytesAvailable() );

    // something has been came back, so do what the
    // state tells us...
    
    QString cmd;
    if ( state == Mail ) {
	cmd = "MAIL FROM:" + from + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Rcpt ) {
	cmd = "RCPT TO:" + to + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Data ) {
	cmd = "DATA\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == From ) {
	cmd = "From: " + from + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == To ) {
	cmd = "To: " + to + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Subject ) {
	cmd = "Subject: " + subject + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Cc ) {
	if ( !cc.isEmpty() ) {
	    cmd = "CC: " + cc + "\r\n";
	    socket->writeBlock( cmd, cmd.length() );
	}
    } else if ( state == Bcc ) {
	if ( !bcc.isEmpty() ) {
	    cmd = "BCC: " + bcc + "\r\n";
	    socket->writeBlock( cmd, cmd.length() );
	}
    } else if ( state == Message ) {
	cmd = message + "\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == DataEnd ) {
	cmd = ".\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Quit ) {
	cmd = "QUIT\r\n";
	socket->writeBlock( cmd, cmd.length() );
    } else if ( state == Close ) {
	socket->close();
	emit finished();
	// delete myself in 1 ms
	QTimer::singleShot( 1, this, SLOT( deleteMe() ) );
    }
    
    // chage state to the next one
    state++;
    
    // if the state is one of these, nothing will come back from
    // the server, so we need to call this slot manually, but
    // to avoid blocking the app, process some events before
    if ( state == From || state == To || state == Subject ||
	 state == Cc || state == Bcc || state == Message ) {
	qApp->processEvents();
	readyRead();
    }
}

void Smtp::deleteMe()
{
    delete this;
}
