/****************************************************************************
** $Id: //depot/qt/main/tests/layout/jex.cpp#1 $
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


#include <qptrdict.h>


struct Container
{
    Container( int r, int c ) :row(r), col(c) {} 
    int row;
    int col;
};
     

class SimpleGrid : public QLayout
{
public:
    SimpleGrid( QWidget *parent, int cols, int border=0, int autoBorder=-1,
	     const char *name=0 );
    SimpleGrid( int cols, int autoBorder=-1, const char *name=0 );

    void add( QWidget *w );
    //    void add( QLayout *layout);
    QSize sizeHint();
protected:
    bool removeWidget( QWidget *w );
    void setGeometry( const QRect& );
private:
    void init();
    int row, col;
    int rr, cc;
    QPtrDict<Container> dict;
};

SimpleGrid::SimpleGrid( QWidget *parent, int cols, int border, int autoBorder,
			const char *name )
    : QLayout( parent, border, autoBorder, name )
{
    init();
    cc = cols;
}
SimpleGrid::SimpleGrid( int cols, int autoBorder=-1, const char *name=0 )
    : QLayout( autoBorder, name )
{
    init();
    cc = cols;
}

void SimpleGrid::init()
{
    row = col = rr = cc = 0;
    dict.setAutoDelete( TRUE );
}

QSize SimpleGrid::sizeHint() { return QSize(0,0); }

bool SimpleGrid::removeWidget( QWidget *w )
 { return dict.remove( w ); }

void SimpleGrid::add( QWidget *w )
{
    if ( cc < 1 ) cc = 1;
     if ( col >= cc ) {
	col = 0;
	row++;
	rr = QMAX( rr, row+1 );
    }
    dict.insert( w, new Container( row, col ) );
    col++;
}

void SimpleGrid::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    if ( rr <= 0 || cc <= 0 )
	return;
    int x0 = r.x();
    int y0 = r.y();

    

    int w = r.width() + defaultBorder();
    int h = r.height() + defaultBorder();
    int cw = w / cc - defaultBorder();
    int ch = h / rr - defaultBorder();
    
    
    QPtrDictIterator<Container> it(dict);
    Container *p;
    while ( (p=it.current()) != 0 ) {
	QWidget *wid = (QWidget*)it.currentKey();
	++it; 
	int x = x0 + ( w * p->col ) / cc;
	int y = y0 + ( h * p->row ) / rr;
	wid->setGeometry( x, y, cw, ch );
	
    }
}

#define TOPLEVEL
int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
#ifndef TOPLEVEL
    QBoxLayout *gm = new QBoxLayout( f, QBoxLayout::BottomToTop, 5 );

    SimpleGrid *b1 = new SimpleGrid( 2 );
    gm->addLayout( b1, 10 );
#else    
    SimpleGrid *b1 = new SimpleGrid( f, 2, 5 );
#endif
    for ( int i=0; i<=4; i++ ) {
	QLabel* lab = new QLabel(f);
	QString s;
	s.sprintf( "Test %d", i );
	lab->setText( s );
	lab->resize( 90 - i*5, 20 + i*10 );
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
	lab->setBackgroundColor(Qt::yellow);
	b1->add(lab);
    }
    QPushButton* qb = new QPushButton( "Quit", f );
    //    qb->setFixedSize( qb->sizeHint() );
    a.connect( qb, SIGNAL(clicked()), SLOT(quit()) );
    b1->add( qb );

#ifndef TOPLEVEL
    QLabel* large = new QLabel(f);
    large->setText("large \nwindow\n you\n know.");
    large->adjustSize();
    large->setMinimumSize( large->size());
    large->setFrameStyle( QFrame::Panel | QFrame::Plain );
    large->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    large->setBackgroundColor(Qt::white);

    QBoxLayout *b2 = new QBoxLayout( QBoxLayout::LeftToRight);
    gm->addLayout( b2, 20 );

    b2->addWidget( large, 100 ); // hstretch

    {
	QLabel* s = new QLabel(f);
	s->setText("This\n is\n supposed\n to be\n centered\n relatively.");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::cyan);
	b2->addWidget( s, 10, Qt::AlignCenter );
    }

    {
	QLabel* s = new QLabel(f);
	s->setText("outermost box");
	s->setFrameStyle( QFrame::Panel | QFrame::Plain );
	s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
	s->setBackgroundColor(Qt::red);
	gm->addWidget( s, 1 );
    }
#endif
    f->show();

    a.setMainWidget(f);
    return a.exec();

}
