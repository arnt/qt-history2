/****************************************************************************
** $Id: //depot/qt/main/tests/layout/fex.cpp#3 $
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
	: QLayout( parent, border, autoBorder, name ),
	cached_width(0) {}
    SimpleFlow( int autoBorder=-1, const char *name=0 )
	: QLayout( autoBorder, name ),
	cached_width(0) {}

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const { return TRUE; }
    int heightForWidth( int ) const;
    QSize sizeHint() const { return QSize(0,0); }
protected:
    bool removeWidget( QWidget *w );
    void setGeometry( const QRect& );
private:
    int layout( const QRect&, bool testonly = FALSE );
    QList<QLayoutItem> list;
    int cached_width;
    int cached_hfw;
};


int SimpleFlow::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
	//Not all C++ compilers support "mutable" yet:
	SimpleFlow * mthis = (SimpleFlow*)this;
	int h = mthis->layout( QRect(0,0,w,0), TRUE );
	mthis->cached_hfw = h;
	return h;
    }
    return cached_hfw;
}


void SimpleFlow::addItem( QLayoutItem *item)
{
    list.append( item );
}

bool SimpleFlow::removeWidget( QWidget *w )
{
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
	++it;
	switch ( o->removeW( w ) ) {
	case Found:
	    return TRUE;
	case FoundAndDeleteable:
	    list.removeRef( o );
	    return TRUE;
	case NotFound:
	    break;
	}
    }
    return FALSE;
}

void SimpleFlow::setGeometry( const QRect &r )
{
    layout( r );
}

int SimpleFlow::layout( const QRect &r, bool testonly )
{
    QLayout::setGeometry( r );
    int x = r.x();
    int y = r.y();
    int h = 0;		//height of this line so far.
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
	++it;
	int nextX = x + o->sizeHint().width() + defaultBorder();
	if ( nextX > r.right() && h > 0 ) { //don't wrap if this line is empty
	    x = r.x();
	    y = y + h + defaultBorder();
	    nextX = x + o->sizeHint().width() + defaultBorder();
	    h = 0;
	}
	if ( !testonly )
	    o->setGeometry( QRect( QPoint( x, y ), o->sizeHint() ) );
	x = nextX;
	h = QMAX( h,  o->sizeHint().height() );
    }
    return y + h - r.y() + 2*margin();
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QGroupBox *f = new QGroupBox;
    f->setFrameStyle( QFrame::Panel  | QFrame::Sunken );
    QBoxLayout *gm = new QVBoxLayout( f, 5 );

    SimpleFlow *b1 = new SimpleFlow;
    gm->addLayout( b1 );
    for ( int i=0; i<4; i++ ) {
	QLabel* lab = new QLabel(f);
	QString s;
	s.sprintf( "Test %d", i );
	lab->setText( s );
	lab->setFixedSize( 90 - i*5, 20 + i*10 );
	lab->setFrameStyle( QFrame::Panel   | QFrame::Plain );
	lab->setAlignment( Qt::AlignTop | Qt::AlignHCenter );
	lab->setBackgroundColor(Qt::yellow);
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

    f->show();

    a.setMainWidget(f);
    return a.exec();

}
