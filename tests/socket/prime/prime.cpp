/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    c->connectToHost("195.0.254.22", 17000);
    c->setMode( QSocket::Ascii );
    connect( c, SIGNAL(readyRead()), SLOT(dataArrived()) );
    connect( c, SIGNAL(closed()), SLOT(timeToUpdate()) );
    delete s;
    s = new QTextStream( c );
    QString str = input->text();
    if ( str[0] == '#' ) {
	str = str.mid(1,20);
	int len = str.toInt();
	str = "";
	for ( int i=0; i<len/16 + 1; i++ ) {
	    str += "0123456789abcdef";
	}
	str.truncate(len);
    }
    *s << str.ascii() << "\r\n";
}


void Prime::dataArrived()
{
    qDebug("Prime::dataArrived");
    if ( c == 0 || s == 0 )
	qDebug("Prime::dataArrived: Fatal error c=%p, s=%p", c, s);
    qDebug("stream eof : %d", s->eof());
    qDebug("canreadline: %d", c->canReadLine());
    QString l;
    if ( !s->eof() && c->canReadLine() ) {
	l = c->readLine();
	output->setText( l );
    }
}


void Prime::timeToUpdate()
{
    qDebug("Prime::timeToUpdate");
    delete c;
    c = 0;
    delete s;
    s = 0;
}


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    Prime *mw = new Prime;
    a.setMainWidget( mw );
    mw->setCaption( "prime client" );
    mw->show();
    a.exec();
    delete mw;
    return 0;
}
