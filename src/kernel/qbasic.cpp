/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbasic.cpp#25 $
**
**  Geometry Management
**
**  Created:  960406
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qbasic.h"
#include "qlist.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qbasic.cpp#25 $");



/*!
  \class QBasicManager qbasic.h
  \brief The QBasicManager class provides one-dimensional geometry management.

  This class is intended for those who write geometry managers and
  graphical designers. <strong>It is not for the faint of
  heart. </strong> The QBoxLayout and QGridLayout classes are
  available for normal application programming.

  Each dimension (horizontal and vertical) is handled independently. Widgets
  are organized in chains, which can be parallel or serial.

  In a serial chain, elements are added one after another. Available
  space is divided among the elements according to stretch and
  max-/minsize.

  In parallel chains, elements are added on top of each other. All
  elements are given the full length of the chain, and are placed at
  the same position.


  \sa QBoxLayout QGridLayout 

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

    virtual bool addBranch( QChain*, int, int )
    {
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

    QList<QChain> chain;

    int minMax();
    int maxMin();
};


struct QBranchData {
    int from;
    int to;
    QChain *chain;
};

/*
  If all members have zero stretch, the space is divided equally;
  this is perhaps not what you want. Use setMaximumSize to be sure your
  widget is not stretched.

 */
class QSerChain : public QChain
{
public:

    QSerChain( QBasicManager::Direction d ) : QChain( d ) {}
    ~QSerChain();

    bool addC( QChain *s );
    bool addBranch( QChain*, int, int );

    void recalc();
    void distribute( wDict &, int, int);
    int maxSize() { return maxsize; }
    int minSize() { return minsize; }

private:
    int maxsize;
    int minsize;

    QList<QChain> chain;
    QList<QBranchData> branches;
    int sumMax();
    int sumMin();
    int sumStretch();
};



QParChain::~QParChain()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	delete chain.at(i);
    }
}

void QParChain::distribute( wDict & wd, int pos, int space )
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	chain.at(i)->distribute(  wd, pos, space );
    }
}

QSerChain::~QSerChain()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i++ ) {
	delete chain.at(i);
    }
    for ( i = 0; i < (int)branches.count(); i++ ) {
	delete branches.at(i);
    }
}

//### possible bug if RightToLeft or Up
bool QSerChain::addBranch( QChain *b, int from, int to )
{
    if ( from < 0 || to < from || from >= (int)chain.count() ) {
	warning( "QBasicManager: Invalid anchor for branch" );
	return FALSE;
    }
    if ( horz( direction() ) != horz( b->direction() ) ) {
	warning( "QBasicManager: branch 90 degrees off" );
	return FALSE;
    }
    QBranchData *d = new QBranchData;
    d->chain = b;
    d->from = from;
    d->to = to;
    branches.append( d );
    return TRUE;
}



static inline int toFixed( int i ) { return i * 256; }
static inline int fRound( int i ) { 
    return  i % 256 < 128 ? i / 256 : 1 + i / 256; 
}
/*
  \internal
  This is the main workhorse of the geometry manager. It portions out
  available space to the chain's children.

  The calculation is done in fixed point: "fixed" variables are scaled 
  by a factor of 256.

  If the chain runs "backwards" (i.e. RightToLeft or Up) the layout
  is computed mirror-reversed, and then turned the right way at the end.
  
*/

void QSerChain::distribute( wDict & wd, int pos, int space )
{
    typedef int fixed;

    fixed available = toFixed( space - minSize() );
    if ( available < 0 ) {
	warning( "QBasicManager: not enough space to go around" );
	available = 0;
    }
    int sf = sumStretch();

    QArray<fixed> sizes( chain.count() );
    int i;
    for ( i = 0; i < (int)chain.count(); i++ )
	sizes[i] = 0;
    bool doAgain = TRUE;
    int numChains = chain.count();
    while ( doAgain && numChains ) {
	doAgain = FALSE;
	for ( i = 0; i < (int)chain.count(); i++ ) {
	    fixed maxS = toFixed( chain.at(i)->maxSize() );
	    if ( sizes[i] == maxS )
		continue;
	    fixed minS = toFixed( chain.at(i)->minSize() );
	    fixed siz = minS;
	    if ( sf )
		siz += available * chain.at(i)->stretch() / sf;
	    else
		siz += available  / numChains;
	    if ( siz >=  maxS ) {
		sizes[i] = maxS;
		available -= maxS - minS;
		sf -= chain.at(i)->stretch();
		numChains--;
		doAgain = TRUE;
		break;
	    }
	    sizes[i] = siz;
	}
    }

    int n = chain.count();
    QArray<int> places( n + 1 );
    places[n] = pos + space;
    fixed fpos = toFixed( pos );
    for ( i = 0; i < (int)chain.count(); i++ ) {
	places[i] = QMAX( fRound( fpos ), pos );  // only give what we've got
	fpos += sizes[i];
    }

    bool backwards = ( direction() == QBasicManager::RightToLeft ||
		       direction() == QBasicManager::Up );

    for ( i = 0; i < (int)chain.count(); i++ ) {
	int p = places[i];
	int s = places[i+1] - places[i];
	if ( backwards )
	    p = 2 * pos + space - p - s;
	chain.at(i)->distribute( wd, p, s );
    }

    for ( i = 0; i < (int)branches.count(); i++ ) {
	QBranchData *b = branches.at( i );
	int from = places[ b->from ];
	int to = places[ b->to + 1 ];
	int s = to - from;
	if ( backwards )
	    from = 2 * pos + space - from - s;
	branches.at(i)->chain->distribute( wd, from, s  );
    }
}

void QParChain::recalc()
{
    for ( int i = 0; i < (int)chain.count(); i ++ )
	chain.at(i)->recalc();
    maxsize = minMax();
    minsize = maxMin();
}


int QParChain::maxMin()
{
    int max = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ ) {
	int m = chain.at(i)->minSize();
	if ( m	> max )
	    max = m;
    }
    return max;
}

int QParChain::minMax()
{
    int min = QBasicManager::unlimited;
    for ( int i = 0; i < (int)chain.count(); i ++ ) {
	int m = chain.at(i)->maxSize();
	if ( m < min )
	    min = m;
    }
    return min;
}

void QSerChain::recalc()
{
    int i;
    for ( i = 0; i < (int)chain.count(); i ++ )
	chain.at(i)->recalc();
    for ( i = 0; i < (int)branches.count(); i ++ )
	branches.at(i)->chain->recalc();
    minsize = sumMin();
    maxsize = sumMax();
}


int QSerChain::sumStretch()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	 s += chain.at(i)->stretch();
    return s;
}

int QSerChain::sumMin()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	s += chain.at(i)->minSize();
    return s;
}

int QSerChain::sumMax()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.count(); i ++ )
	s += chain.at(i)->maxSize();
    if ( s > QBasicManager::unlimited )
	s = QBasicManager::unlimited;
    return s;
}



bool QSerChain::addC( QChain *s )
{
    if ( horz( s->direction() ) != horz( direction() ) ) {
	if ( horz( direction() ) )
	    warning("QBasicManager:Cannot add vertical chain to horizontal serial chain");
	else
	    warning("QBasicManager:Cannot add horizontal chain to vertical serial chain");
	return FALSE;
    }
    chain.append( s );
    return TRUE;
}

bool QParChain::addC( QChain *s )
{
    if ( horz( s->direction() ) != horz( direction() ) ) {
	if ( horz( direction() ) )
	    warning("QBasicManager:Cannot add vertical chain to horizontal parallel chain");
	else
	    warning("QBasicManager:Cannot add horizontal chain to vertical parallel chain");
	return FALSE;
    }
    chain.append( s );
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
    frozen = FALSE:

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

bool QBasicManager::activate()
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
    frozen = FALSE; // so activate can do it.
    activate();

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

    QSize max = main->maximumSize();
    QSize min = main->minimumSize();

    // size may not be set yet
    int ww = QMAX( min.width(), QMIN( main->width(), max.width() ) );
    int hh = QMAX( min.height(), QMIN( main->height(), max.height() ) );
    
    xC->distribute( lookupTable, border, ww - 2*border );
    yC->distribute( lookupTable, border, hh - 2*border );

    QIntDictIterator<WidgetInfo> it( lookupTable );

    WidgetInfo *w;
    while (( w = it.current() )) {
	++it;
	if ( w->widget )
	    w->widget->setGeometry( w->geom );
	delete w;
    }
}


/*!

  Adds \a branch to \a destination as a branch going from \a fromIndex
  to \a toIndex. A branch is a chain that is anchored at two locations
  in a serial chain. The branch does not influence the main chain;
  if the branch's minimum size is greater than the minimum distance
  between the anchors, things will look ugly.

  The branch goes from the beginning of the item at \a fromIndex to the
  end of the item at \a toIndex. Note: remember to count spacing when
  calculating indices.
  
  \warning This feature is new and not comprehensively tested.
*/

bool QBasicManager::addBranch( QChain *destination, QChain *branch,
			       int fromIndex, int toIndex )
{
    bool success = destination->addBranch( branch, fromIndex, toIndex );
    if ( ! success )
	warning( "QBasicManager: Couldn't add branch" );
    return success;
}


/*!
  Sets the stretch factor on \a c to \a s. This stretch factor is overridden
  by add(), so it's no point in calling this function before you add() the chain.
  \sa add()
*/

void QBasicManager::setStretch( QChain *c, int s )
{
    c->setStretch( s );
}
