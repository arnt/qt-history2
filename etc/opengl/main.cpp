/****************************************************************************
**
** Nurb demonstration
**
** Copyright (C) 1996 by Troll Tech AS.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#include "nurb.h"
#include <qapplication.h>
#include <qkeycode.h>
#include <qaccel.h>
#include <qpushbutton.h>


class NurbSample : public QWidget {
public:
    NurbSample();
protected:
    void   closeEvent( QCloseEvent * ) { QApplication::exit(0); }
private:
    Nurb *nurb;
};


NurbSample::NurbSample()
{
    nurb = new Nurb( this, 0 );
    nurb->setGeometry( 10, 200, 320, 320 );

    QPushButton *b;

  // Creates some buttons and connects the button's clicked signal
  // to actions in the nurb widget.

    b = new QPushButton( this );
    b->setText( "Left" );
    b->setGeometry( 10, 70, 100, 30 );
    connect( b, SIGNAL(clicked()), nurb, SLOT(doLeft()) );
    b = new QPushButton( this );
    b->setText( "Up" );
    b->setGeometry( 120, 10, 100, 30 );
    connect( b, SIGNAL(clicked()), nurb, SLOT(doUp()) );
    b = new QPushButton( this );
    b->setText( "Right" );
    b->setGeometry( 230, 70, 100, 30 );
    connect( b, SIGNAL(clicked()), nurb, SLOT(doRight()) );
    b = new QPushButton( this );
    b->setText( "Down" );
    b->setGeometry( 120, 130, 100, 30 );
    connect( b, SIGNAL(clicked()), nurb, SLOT(doDown()) );

    b = new QPushButton( this );
    b->setText( "Go!" );
    b->setGeometry( 130, 70, 80, 30 );
    b->setToggleButton( TRUE );
    connect( b, SIGNAL(clicked()), nurb, SLOT(animate()) );

  // Create an accelerator to handle arrow keys

    QAccel *a = new QAccel( this );
    a->connectItem( a->insertItem(Key_Left), nurb,SLOT(doLeft()) );
    a->connectItem( a->insertItem(Key_Right),nurb,SLOT(doRight()) );
    a->connectItem( a->insertItem(Key_Up),   nurb,SLOT(doUp()) );
    a->connectItem( a->insertItem(Key_Down), nurb,SLOT(doDown()) );

    adjustSize();
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( argc > 1 && strcmp(argv[1],"-s")==0 )
	GLWidget::setDoubleBuffer( FALSE );

    NurbSample *w;
    w = new NurbSample;
    w->show();
    w = new NurbSample;
    w->show();
    w = new NurbSample;
    w->show();

    return a.exec();
}
