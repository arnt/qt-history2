/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "smtp.h"

#include <qsocket.h>
#include <qtimer.h>
#include <qapplication.h>

Smtp::Smtp( const QString &server_, int port_,
	    const QString &from_, const QString &to_, const QString &subject_,
	    const QString &cc_, const QString &bcc_, const QString &message_ )
    : server( server_ ), from( from_ ), to( to_ ), subject( subject_ ),
      cc( cc_ ), bcc( bcc_ ), message( message_ ), port( port_ )
{
    socket = new QSocket( this );
    connect ( socket, SIGNAL( readyRead() ),
	      this, SLOT( readyRead() ) );
    connect ( socket, SIGNAL( connected() ),
	      this, SLOT( connected() ) );

    socket->connectToHost( server, port );
    state = Init;
}

void Smtp::connected()
{
    QString cmd = "HELO " + server + "\r\n";
    socket->writeBlock( cmd, cmd.length() );
    state = Mail;
}

void Smtp::readyRead()
{
    QCString s;
    s.resize( socket->bytesAvailable() );
    socket->readBlock( s.data(), socket->bytesAvailable() );

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
	QTimer::singleShot( 1, this, SLOT( deleteMe() ) );
    }
    state++;
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
