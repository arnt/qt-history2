/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbasic.cpp#18 $
**
**  Studies in Geometry Management
**
**  Created:  960406
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qbasic.h"
#include "qlist.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qbasic.cpp#18 $");



/*!
  \class QBasicManager qbasic.h
  \brief The QBasicManager class provides one-dimensional geometry management.

  This class is intended for those who write geometry managers and
  graphical designers.	It is not for the faint of heart. The
  QBoxLayout class is available for normal application programming.

  Each dimension (horizontal and vertical) is handled independently. Widgets
  are organized in chains, which can be parallel or serial.

  \sa QBoxLayout
*/


static inline bool horz( QBasicManager::Direction dir )
{
    return dir == QBasicManager::RightToLeft || dir == QBasicManager::LeftToRight;
}
struct WidgetInfo {
    QRect geom;
    QWidget *widget;
};

typedef QIntDict<WidgetInfo> wDict;


WidgetInfo *lookup( QWidget * w, wDict & table,
		    bool create = FALSE )
{
    WidgetInfo *wi = table[ (long) w ];
    if ( !wi && create ) {
	wi = new WidgetInfo;
	wi->widget = w;
	table.insert( (long) w, wi );
    }
    return wi;
}




static void setWinfo( QWidget * w, wDict &dict, QBasicManager::Direction d, int p, int s )
{
    WidgetInfo *wi = lookup( w, dict, TRUE );
    if ( horz( d ) ) {
	wi->geom.setX( p );
	wi->geom.setWidth( s );
    } else {
	wi->geom.setY( p );
	wi->geom.setHeight( s );
    }
}

/*!
  \internal
  \class QChain
  \brief internal class for the QBasicManager.

  Everything is put into chains. Use QBasicManager::newParChain()
  or QBasicManager::newSerChain() to make chains.

  \sa QBasicManager.
*/
class QChain
{
public:

    QChain( QBasicManager::Direction d ) { dir = d; }
    virtual ~QChain() {}

    bool add( QChain *s, int stretch )
    {
	if ( addC(s) ) {
	    s->sstretch = stretch;
	    return TRUE;
	} else
	    return FALSE;
    }

    virtual int maxSize() = 0;
    virtual int minSize() = 0;
    int stretch() { return sstretch; }
    void setStretch( int s ) { sstretch = s; }
    virtual void recalc() {}

    virtual void distribute( wDict&, int pos, int space) = 0;

    QBasicManager::Direction direction() { return dir; }

protected:
    virtual bool addC( QChain *s ) = 0;
private:
    QBasicManager::Direction dir;

    int sstretch;
};


class QSpaceChain : public QChain
{
public:
    QSpaceChain( QBasicManager::Direction d, int min, int max )
	: QChain( d ), minsize( min ), maxsize( max ) {}
    // needs direction for consistency check.....
    bool addC( QChain * ) { return FALSE; }


    void distribute( wDict&, int, int ) {}

    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

private:
    int minsize;
    int maxsize;
};

class QWidChain : public QChain
{
public:
    QWidChain( QBasicManager::Direction d,  QWidget * w )
	: QChain( d ), widget ( w ) {}
    bool addC( QChain * ) { return FALSE; }

    int minSize()
    {
	QSize s = widget->minimumSize();
	if ( horz( direction() ) )
	    return s.width();
	else
	    return s.height();
    }
    int maxSize()
    {
	QSize s = widget->maximumSize();
	if ( horz( direction() ) )
	    return s.width();
	else
	    return s.height();
    }

    void distribute( wDict & wd, int pos, int space ) {
	setWinfo( widget, wd, direction(),  pos, space );
    }

private:
    QWidget * widget;

};

class QParChain : public QChain
{
public:

    QParChain( QBasicManager::Direction d )
	: QChain( d )
    {
    }

    ~QParChain();
    bool addC( QChain *s );

    void recalc();

    void distribute( wDict &, int, int );

    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

private:
    int maxsize;
    int minsize;
    int sstretch;

    QArray<QChain*> chain;

    int minMax();
    int maxMin();
};

class QSerChain : public QChain
{
public:

    QSerChain( QBasicManager::Direction d ) : QChain( d ) {}
    ~QSerChain();

    bool addC( QChain *s );

    void recalc();

    void distribute( wDict &, int, int);

    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

private:
    int maxsize;
    int minsize;

    QArray<QChain*> chain;
    int sumMax();
    int sumMin();
    int sumStretch();
};


QParChain::~QParChain()
{
    int i;
    for ( i = 0; i < (int)chain.size(); i++ ) {
	delete chain[i];
    }
}

void QParChain::distribute( wDict & wd, int pos, int space )
{
    int i;
    for ( i = 0; i < (int)chain.size(); i++ ) {
	chain[i]->distribute(  wd, pos, space );
    }
}


QSerChain::~QSerChain()
{
    int i;
    for ( i = 0; i < (int)chain.size(); i++ ) {
	delete chain[i];
    }
}


/*
  If all members have zero stretch, the space is divided equally;
  this is slightly evil... Use setMaximumSize to be sure your
  widget is not stretched.
*/

void QSerChain::distribute( wDict & wd, int pos, int space )
{
    bool backwards = direction() == QBasicManager::RightToLeft ||
       direction() == QBasicManager::Up;

    int available = space - minSize();
    if ( available < 0 )
	available = 0;
    int sf = sumStretch();

    QArray<int> size( chain.size() );
    int i;
    for ( i = 0; i < (int)chain.size(); i++ )
	size[i] = 0;
    bool doAgain = TRUE;
    int numChains = chain.size();
    while ( doAgain && numChains ) {
	doAgain = FALSE;
	for ( i = 0; i < (int)chain.size(); i++ ) {
	    if ( size[i] == chain[i]->maxSize() )
		continue;
	    int siz = chain[i]->minSize();
	    if ( sf )
		siz += ( available * chain[i]->stretch() ) / sf;
	    else
		siz += available  / numChains;
	    if ( siz >= chain[i]->maxSize() ) {
		size[i] = chain[i]->maxSize();
		available -= ( size[i] - chain[i]->minSize() );
		sf -= chain[i]->stretch();
		numChains--;
		doAgain = TRUE;
		break;
	    }
	    size[i] = siz;
	}
    }
    if ( backwards )
	pos += space;
    for ( i = 0; i < (int)chain.size(); i++ ) {
	if ( backwards ) {
	    pos -= size[i];
	    chain[i]->distribute( wd, pos, size[i] );
	} else {
	    chain[i]->distribute( wd, pos, size[i] );
	    pos += size[i];
	}

    }

}

void QParChain::recalc()
{
    for ( int i = 0; i < (int)chain.size(); i ++ )
	chain[i]->recalc();
    maxsize = minMax();
    minsize = maxMin();
}


int QParChain::maxMin()
{
    int max = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ ) {
	int m = chain[i]->minSize();
	if ( m	> max )
	    max = m;
    }
    return max;
}

int QParChain::minMax()
{
    int min = QBasicManager::unlimited;
    for ( int i = 0; i < (int)chain.size(); i ++ ) {
	int m = chain[i]->maxSize();
	if ( m < min )
	    min = m;
    }
    return min;
}

void QSerChain::recalc()
{
    for ( int i = 0; i < (int)chain.size(); i ++ )
	chain[i]->recalc();
    minsize = sumMin();
    maxsize = sumMax();
}


int QSerChain::sumStretch()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	 s += chain[i]->stretch();
    return s;
}

int QSerChain::sumMin()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	s += chain[i]->minSize();
    return s;
}

int QSerChain::sumMax()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	s += chain[i]->maxSize();
    if ( s > QBasicManager::unlimited )
	s = QBasicManager::unlimited;
    return s;
}



bool QSerChain::addC( QChain *s )
{
     if ( horz( s->direction() ) != horz( direction() ) )
	return FALSE;
    int n = chain.size();
    chain.resize( n + 1 );
    chain[n] = s;
    return TRUE;
}

bool QParChain::addC( QChain *s )
{
     if ( horz( s->direction() ) != horz( direction() ) )
	return FALSE;
    int n = chain.size();
    chain.resize( n + 1 );
    chain[n] = s;
    return TRUE;
}

/*!
  Creates a new QBasicManager which manages \e parent's children.
*/
QBasicManager::QBasicManager( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    main = parent;
    border = 0;

    xC = new QParChain( LeftToRight );
    yC = new QParChain( Down );

    if ( parent ) {
	parent->installEventFilter( this );
    }
}

/*!
  Destroys the QBasicManager, deleting all add()ed chains.
*/
QBasicManager::~QBasicManager()
{
    delete xC;
    delete yC;
}


/*!
  \fn QChain *QBasicManager::xChain()

  Returns the main horizontal chain of the manager. All horizontal chains
  should be inserted into this chain or one of its descendants, otherwise
  they will be ignored.
*/

/*!
  \fn QChain *QBasicManager::yChain()

  Returns the main vertical chain of the manager. All vertical chains
  should be inserted into this chain or one of its descendants, otherwise
  they will be ignored.
*/


/*!
  \fn void QBasicManager::setBorder( int b )

  Sets the border around the edge of the widget. \e b is the number of
  pixels between the edge of the widget and the area controlled by the
  manager.
*/


/*!
  Creates a new QChain which is \e parallel.
*/

QChain * QBasicManager::newParChain( Direction d )
{
    QChain * c = new QParChain( d );
    CHECK_PTR(c);
    return c;
}


/*!
  Creates a new QChain which is \e serial.
*/

QChain * QBasicManager::newSerChain( Direction d )
{
    QChain * c = new QSerChain( d );
    CHECK_PTR(c);
    return c;
}

/*!
  Adds the chain \e source to the chain \e destination.
*/

bool QBasicManager::add( QChain *destination, QChain *source, int stretch )
{
    return destination->add(source, stretch);
}


/*!
  Adds the widget  \e w to the chain \e d.
*/

bool QBasicManager::addWidget( QChain *d, QWidget *w, int stretch )
{
    //if ( w->parent() != main ) {
    //	  warning("QBasicManager::addWidget - widget is not child.");
    //	  return FALSE;
    //}
    return d->add( new QWidChain( d->direction(), w) , stretch );
}

/*!
  Adds the spacing  \e w to the chain \e d. If \e d is a serial chain, this
  means screen space between widgets. If \e d is parallel, this influences
  the maximum and minimum size.
*/

bool QBasicManager::addSpacing( QChain *d, int minSize, int stretch, int maxSize )
{
    return d->add( new QSpaceChain( d->direction(), minSize, maxSize), stretch	);
}

/*!
  Grabs all resize events for my parent, and does child widget resizing.
*/

bool QBasicManager::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->inherits( "QWidget" ))
	return FALSE;

    QWidget *w = (QWidget*)o;

    if ( e->type() == Event_Resize ) {
	QResizeEvent *r = (QResizeEvent*)e;
	resizeHandle( w, r->size() );
	return TRUE;			    // eat event
    }

    return FALSE;			    // standard event processing
}

void QBasicManager::resizeHandle( QWidget *, const QSize & )
{
    resizeAll();
}

/*!
  Starts geometry management.
*/

bool QBasicManager::doIt()
{
    if ( frozen )
	return FALSE;

    yC->recalc();
    xC->recalc();

    int ys = yC->minSize() + 2*border;
    int xs = xC->minSize() + 2*border;

    main->setMinimumSize( xs, ys );

    ys = yC->maxSize() + 2*border;
    if ( ys > QBasicManager::unlimited )
	ys = QBasicManager::unlimited;
    xs = xC->maxSize() + 2*border;
    if ( xs > QBasicManager::unlimited )
	xs = QBasicManager::unlimited;

    main->setMaximumSize( xs, ys );

    resizeAll(); //### double recalc...
    return TRUE;
}

/*!
  Fixes the size of the main widget and distributes the available
  space to the child widgets. The size is adjusted to a valid
  value. Thus freeze(0,0) (the default) will fix the widget to its
  minimum size.
*/
void QBasicManager::freeze( int w, int h )
{
    frozen = FALSE; // so doIt can do it.
    doIt();

    QSize max = main->maximumSize();
    QSize min = main->minimumSize();

    w = QMAX( min.width(), QMIN( w, max.width() ) );
    h = QMAX( min.height(), QMIN( w, max.height() ) );
    main->setMaximumSize( w, h );
    main->setMinimumSize( w, h );
    main->resize( w, h );

    frozen = TRUE;
}

void QBasicManager::resizeAll()
{

    QIntDict<WidgetInfo> lookupTable;

    xC->recalc();
    yC->recalc();
    xC->distribute( lookupTable, border, main->width() - 2*border );
    yC->distribute( lookupTable, border, main->height() - 2*border );

    QIntDictIterator<WidgetInfo> it( lookupTable );

    WidgetInfo *w;
    while (( w = it.current() )) {
	++it;
	if ( w->widget )
	    w->widget->setGeometry( w->geom );
	delete w;
    }
}
