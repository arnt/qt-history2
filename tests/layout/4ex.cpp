/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** EDITIONS: UNKNOWN
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <qwidget.h>

class Kill : public QWidget
{
public:
    Kill( QWidget *parent=0, const char *name=0 )
	:QWidget(parent,name) {}
    QSize sizeHint() const { return QSize(20,20); }
protected:
    void mouseReleaseEvent( QMouseEvent * );
};

#include <qevent.h>
#include <qapplication.h>

void Kill::mouseReleaseEvent( QMouseEvent *m )
{
    QWidget *w = QApplication::widgetAt( mapToGlobal( m->pos() ),TRUE);
    if ( w && w != this && w != parentWidget() )
	delete w;
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QFrame *f = new QFrame;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );

    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::TopToBottom, 5 );

    QBoxLayout *b1 = new QBoxLayout( QBoxLayout::LeftToRight );
    gm->addLayout( b1, 10 );

    for ( int i=0; i<4; i++ ) {

	if ( i != 2 ) {
	    QLabel* lab = new QLabel("Testing");
	    lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	    lab->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
	    lab->setBackgroundColor(Qt::yellow);
	    b1->addWidget( lab, 20 );
	} else {
	    QString s;
	    QGridLayout *grid = new QGridLayout( 2, 2 );
	    b1->addLayout( grid );
	    for ( int j = 0; j < 2; j++ )
		for ( int k = 0; k < 2; k++ ) {
		    s.sprintf( "Grid %d,%d", j, k );
		    QLabel* lab = new QLabel(s);
		    lab->setFrameStyle( QFrame::Panel | QFrame::Plain );
		    lab->setAlignment( Qt::AlignCenter );
		    lab->setBackgroundColor(Qt::cyan);
		    grid->addWidget( lab, j, k );
		}
	}
    }

    Kill* kill = new Kill;
    b1->addWidget( kill );
    kill->setBackgroundColor( Qt::red );

    QPushButton* qb = new QPushButton("Quit");
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    b1->addWidget( qb, 0, Qt::AlignTop );

    QLabel* large = new QLabel;
    large->setText("This is supposed to be a large window\n you know.");
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    large->setBackgroundColor(Qt::white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 50 );
    b2->addWidget( large, 100 ); // hstretch

    QLabel* s = new QLabel;
    s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
    s->setFrameStyle( QFrame::Panel | QFrame::Plain );
    s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    s->setBackgroundColor(Qt::cyan);
    b2->addWidget( s, 10, Qt::AlignCenter );

    s = new QLabel;
    s->setText("This is a widget inside the outermost box");
    s->setFrameStyle( QFrame::Panel | QFrame::Plain );
    s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    s->setBackgroundColor(Qt::red);
    gm->addWidget( s, 1 );


    f->show();
    a.setMainWidget(f);
    return a.exec();
}
