/****************************************************************************
** $Id: //depot/qt/main/tests/layout/mex.cpp#3 $
**
** QBoxLayout example with a QMenuBar
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpushbutton.h>

#include <qlayout.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget *f = new QWidget;

    QMenuBar *m = new QMenuBar( f );
    m->insertItem("These");
    m->insertItem("Menus");
    m->insertItem("Do not");
    m->insertItem("Any");
    m->insertItem("Have");
    m->insertItem("Popups");
    m->insertItem("So do not");
    m->insertItem("Bother");
    m->insertItem("Clicking");
    m->insertItem("On them");

    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::TopToBottom, 5 ); 
    gm->setMenuBar( m ); // You MUST tell the layout about the menubar.

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
