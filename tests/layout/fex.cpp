/****************************************************************************
** $Id: //depot/qt/main/tests/layout/fex.cpp#2 $
**
** Implementing your own layout: flow example
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


#include <qlist.h>

class SimpleFlow : public QLayout
{
public:
    SimpleFlow( QWidget *parent, int border=0, int autoBorder=-1,
	     const char *name=0 )
	: QLayout( parent, border, autoBorder, name ) {}
    SimpleFlow( int autoBorder=-1, const char *name=0 )
	: QLayout( autoBorder, name ) {}

    void add( QWidget *w ) { list.append( w ); }
    //    void add( QLayout *layout);
    QSize minSize() { return QSize(0,0); }
protected:
    bool removeWidget( QWidget *w ) { return list.removeRef( w ); }
    void setGeometry( const QRect& );
private:
    QList<QWidget> list;
};

void SimpleFlow::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    int x = r.x();
    int y = r.y();
    int h = 0;		//height of this line so far.
    QListIterator<QWidget> it(list);
    QWidget *w;
    while ( (w=it.current()) != 0 ) {
	++it;
	int nextX = x + w->width() + defaultBorder();
	if ( nextX > r.right() && h > 0 ) { //don't wrap if this line is empty
	    x = r.x();
	    y = y + h + defaultBorder();
	    nextX = x + w->width() + defaultBorder();
	    h = 0;
	}
	w->move( x, y );
	x = nextX;
	h = QMAX( h, w->height() );
    }
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::BottomToTop, 5 );

    SimpleFlow *b1 = new SimpleFlow;
    gm->addLayout( b1, 10 );
    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(f);
	QString s;
	s.sprintf( "Test %d", i );
	lab->setText( s );
	lab->resize( 90 - i*5, 20 + i*10 );
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( AlignTop | AlignHCenter );
	lab->setBackgroundColor(QColor::yellow);
	b1->add(lab);
    }
    QPushButton* qb = new QPushButton( "Quit", f );
    qb->setFixedSize( qb->sizeHint() );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    b1->add( qb );

    QLabel* large = new QLabel(f);
    large->setText("large \nwindow\n you\n know.");
    large->adjustSize();
    large->setMinimumSize( large->size());
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( AlignVCenter | AlignHCenter );
    large->setBackgroundColor(QColor::white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 20 );

    b2->addWidget( large, 100 ); // hstretch

    {
	QLabel* s = new QLabel(f);
	s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(QColor::cyan);
	b2->addWidget( s, 10, AlignCenter );
    }

    {
	QLabel* s = new QLabel(f);
	s->setText("outermost box");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( AlignVCenter | AlignHCenter );
	s->setBackgroundColor(QColor::red);
	gm->addWidget( s, 1 );
    }

    f->show();

    a.setMainWidget(f);
    return a.exec();

}
