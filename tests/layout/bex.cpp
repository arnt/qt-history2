/****************************************************************************
** $Id: //depot/qt/main/tests/layout/bex.cpp#4 $
**
** QBoxLayout example: boxes that go "backwards" (i.e. RightToLeft or
** BottomToTop)
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::BottomToTop, 5 );

    QBoxLayout *b1 = new QBoxLayout( QBoxLayout::RightToLeft);
    gm->addLayout( b1, 10 );

    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(f);
	QString s;
	s.sprintf( "Test %d", i );
	lab->setText( s );
	lab->adjustSize();
	lab->setMinimumSize( lab->size());
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
	lab->setBackgroundColor(Qt::yellow);
	lab->setMaximumSize( 150, 200 );

	b1->addWidget( lab, 20 );
    }


    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    qb->setFixedSize( qb->size() );
    b1->addWidget( qb, 0, Qt::AlignTop );


    QLabel* large = new QLabel(f);
    large->setText("This is supposed to be a large window\n you know.");
    large->adjustSize();
    large->setMinimumSize( large->size());
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    large->setBackgroundColor(Qt::white);

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
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::cyan);
	b2->addWidget( s, 10, Qt::AlignCenter );
    }

    {
	QLabel* s = new QLabel(f);
	s->setMaximumSize( QLayout::unlimited, 50 );
	s->setText("This is a widget inside the outermost box");
	s->adjustSize();
	s->setMinimumSize( s->width(), s->height() );
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::red);
	gm->addWidget( s, 1 );
    }

    gm->activate();
    //gm->freeze();
    f->show();

    a.setMainWidget(f);
    return a.exec();
}
