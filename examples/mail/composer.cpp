/****************************************************************************
** $Id: //depot/qt/main/examples/qfileiconview/main.cpp#10 $
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "composer.h"
#include "smtp.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvalidator.h>

Composer::Composer( QWidget *parent )
    : QVBox( parent )
{
    // Setup the GUI of the composer
    
    setSpacing( 5 );
    setMargin( 5 );

    QHBox *row = new QHBox( this );
    row->setSpacing( 5 );

    (void)new QLabel( tr( "Outgoing Mailserver (SMTP):" ), row );
    server = new QLineEdit( row );

    (void)new QLabel( tr( "Port:" ), row );
    port = new QLineEdit( row );
    port->setValidator( new QIntValidator( port ) );
    port->setText( "25" );
    port->setFixedWidth( port->sizeHint().width() / 4 );

    QPushButton *send = new QPushButton( tr( "&Send" ), row );
    connect( send, SIGNAL( clicked() ), this, SLOT( sendMessage() ) );

    row = new QHBox( this );
    row->setSpacing( 5 );
    (void)new QLabel( tr( "From:" ), row );
    from = new QLineEdit( row );

    row = new QHBox( this );
    row->setSpacing( 5 );
    (void)new QLabel( tr( "To:" ), row );
    to = new QLineEdit( row );

    row = new QHBox( this );
    row->setSpacing( 5 );
    (void)new QLabel( tr( "Subject:" ), row );
    subject = new QLineEdit( row );

    row = new QHBox( this );
    row->setSpacing( 5 );
    (void)new QLabel( tr( "CC:" ), row );
    cc = new QLineEdit( row );

    row = new QHBox( this );
    row->setSpacing( 5 );
    (void)new QLabel( tr( "Bcc:" ), row );
    bcc = new QLineEdit( row );

    message = new QMultiLineEdit( this );
}

void Composer::sendMessage()
{
    setEnabled( FALSE );

    // send the mail
    Smtp *smtp = new Smtp( server->text(), port->text().toInt(), from->text(), to->text(),
			   subject->text(), cc->text(), bcc->text(), message->text() );
    connect( smtp, SIGNAL( finished() ),
	     this, SLOT( enableAll() ) );
}

void Composer::enableAll()
{
    setEnabled( TRUE );
}
