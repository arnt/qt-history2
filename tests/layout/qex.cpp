/****************************************************************************
** $Id: //depot/qt/main/tests/layout/qex.cpp#1 $
**
** QBoxLayout example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapp.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgrpbox.h> 
#include <qpushbt.h>

#include <qlayout.h>

RCSTAG("$Id: //depot/qt/main/tests/layout/qex.cpp#1 $");

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::TopToBottom, 5 ); 

    QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b1, 10 );

    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(f);
	lab->setText("Testing");
	lab->adjustSize();
	lab->setMinimumSize( lab->size());
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( AlignTop | AlignHCenter );
	lab->setBackgroundColor(yellow);
	lab->setMaximumSize( 150, 200 );

	b1->addWidget( lab, 20 );
    }


    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    qb->setFixedSize( qb->size() );
    b1->addWidget( qb, 0, AlignTop );
    

    QLabel* large = new QLabel(f);
    large->setText("This is supposed to be a large window\n you know.");
    large->adjustSize();
    large->setMinimumSize( large->size());
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( AlignVCenter | AlignHCenter );
    large->setBackgroundColor(white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 50 );

    b2->addWidget( large, 100 ); // hstretch

    {
	QLabel* s = new QLabel(f);
	s->setMaximumSize(150,150);
	s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
	s->adjustSize();
	s->setMinimumSize( s->size());
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(cyan);
	b2->addWidget( s, 10, AlignCenter );
    }

    {
	QLabel* s = new QLabel(f);
	s->setMaximumSize( QLayout::unlimited, 50 );
	s->setText("This is a widget inside the outermost box");
	s->adjustSize();
	s->setMinimumSize( s->width(), s->height() );
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(red);
	gm->addWidget( s, 1 );
    }

    gm->activate();
    //gm->freeze();
    f->show();

    a.setMainWidget(f);
    return a.exec();
}

