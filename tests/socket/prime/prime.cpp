/****************************************************************************
** $Id: //depot/qt/main/tests/socket/prime/prime.cpp#1 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "prime.h"

#include <qlayout.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qtextstream.h>
#include <qpushbutton.h>
#include <qlabel.h>


Prime::Prime( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    input = new QLineEdit( this, "number" );
    input->setText( "17" );
    connect( input, SIGNAL(returnPressed()), SLOT(makeConnection()) );

    QPushButton * quit = new QPushButton( "Quit", this );
    quit->setFixedSize( quit->sizeHint() );
    connect( quit, SIGNAL(clicked()), qApp, SLOT(quit()) );

    output = new QLabel( this );

    QBoxLayout * l = new QBoxLayout( this, QBoxLayout::Up );
    l->addWidget( output, 1 );
    l->addSpacing( 3 );

    QBoxLayout * l2 = new QBoxLayout( QBoxLayout::LeftToRight );
    l->addLayout( l2, 0 );

    l2->addWidget( input, 1 );
    l2->addSpacing( 3 );
    l2->addWidget( quit );

    c = 0;
    s = 0;
}


Prime::~Prime()
{
    delete c;
    delete s;
}


void Prime::makeConnection()
{
    QString tmp;
    tmp.sprintf( "Asking about %s", input->text().ascii() );
    output->setText( tmp );

    delete c;
    c = new QSocket;
    c->connectToHost("localhost", 17000);
    c->setMode( QSocket::Ascii );
    connect( c, SIGNAL(readyRead()), SLOT(dataArrived()) );
    connect( c, SIGNAL(closed()), SLOT(timeToUpdate()) );
    delete s;
    s = new QTextStream( c );
    *s << input->text() << "\r\n";
}


void Prime::dataArrived()
{
    debug("Prime::dataArrived");
    debug("stream eof : %d", s->eof());
    debug("canreadline: %d", c->canReadLine());
    QString l;
    if ( !s->eof() && c->canReadLine() ) {
	l = c->readLine();
	output->setText( l );
    }
}


void Prime::timeToUpdate()
{
    delete c;
    c = 0;
    delete s;
    s = 0;
}


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Prime mw;
    a.setMainWidget( &mw );
    mw.setCaption( "prime client" );
    mw.show();
    return a.exec();
}
