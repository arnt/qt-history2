/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qbasic.cpp#6 $
**
**  Studies in Geometry Management
**
**  Author:   Paul Olav Tvete
**  Created:  960406
**
** Copyright (C) 1996 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include <qlist.h>

#include <qrect.h>
#include <qevent.h>
#include <qpoint.h>
#include "qbasic.h"


RCSTAG("$Id: //depot/qt/main/src/kernel/qbasic.cpp#6 $")



/*!
  \class QBasicManager qbasic.h
  \brief The QBasicManager class provides one-dimensional geometry management.

  This class is not for the faint of heart. The QGeomManager class is
  available for normal application programming.

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


class QChain
{
public:

    QChain( QBasicManager::Direction d ) { dir = d; }

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


class SpaceChain : public QChain
{
public:
    SpaceChain( QBasicManager::Direction d, int min, int max )
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

class WidChain : public QChain
{
public:
    WidChain( QBasicManager::Direction d,  QWidget * w )
	: QChain( d ), widget ( w ) {}
    bool addC( QChain * ) { return FALSE; }

    int minSize()
    {
	int wid = 0;
	int h = 0;
	widget->minimumSize( &wid, &h );
	if ( horz( direction() ) )
	    return wid;
	else
	    return h;
    }
    int maxSize()
    {
	int wid = QBasicManager::unlimited;
	int h = QBasicManager::unlimited;
	widget->maximumSize( &wid, &h );
	if ( horz( direction() ) )
	    return wid;
	else
	    return h;
    }
    void distribute( wDict & wd, int pos, int space ) {
	setWinfo( widget, wd, direction(),  pos, space );
    }

private:
    QWidget * widget;

};

class ParChain : public QChain
{
public:

    ParChain( QBasicManager::Direction	d )
	: QChain( d )
    {
    }

    ~ParChain() {}
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

class SerChain : public QChain
{
public:

    SerChain( QBasicManager::Direction d ) : QChain( d ) {}
    ~SerChain() {}

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


void ParChain::distribute( wDict & wd, int pos, int space )
{
    int i;
    for ( i = 0; i < (int)chain.size(); i++ ) {
	chain[i]->distribute(  wd, pos, space );
    }
}


/*
  If all members have zero stretch, the space is divided equally;
  this is slightly evil... Use setMaximumSize to be sure your
  widget is not stretched.
*/

void SerChain::distribute( wDict & wd, int pos, int space )
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

void ParChain::recalc()
{
    for ( int i = 0; i < (int)chain.size(); i ++ )
	chain[i]->recalc();
    maxsize = minMax();
    minsize = maxMin();
}


int ParChain::maxMin()
{
    int max = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ ) {
	int m = chain[i]->minSize();
	if ( m	> max )
	    max = m;
    }
    return max;
}

int ParChain::minMax()
{
    int min = QBasicManager::unlimited;
    for ( int i = 0; i < (int)chain.size(); i ++ ) {
	int m = chain[i]->maxSize();
	if ( m < min )
	    min = m;
    }
    return min;
}

void SerChain::recalc()
{
    for ( int i = 0; i < (int)chain.size(); i ++ )
	chain[i]->recalc();
    minsize = sumMin();
    maxsize = sumMax();
}


int SerChain::sumStretch()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	 s += chain[i]->stretch();
    return s;
}

int SerChain::sumMin()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	s += chain[i]->minSize();
    return s;
}

int SerChain::sumMax()
{
    int s = 0;
    for ( int i = 0; i < (int)chain.size(); i ++ )
	s += chain[i]->maxSize();
    if ( s > QBasicManager::unlimited )
	s = QBasicManager::unlimited;
    return s;
}



bool SerChain::addC( QChain *s )
{
     if ( horz( s->direction() ) != horz( direction() ) )
	return FALSE;
    int n = chain.size();
    chain.resize( n + 1 );
    chain[n] = s;
    return TRUE;
}

bool ParChain::addC( QChain *s )
{
     if ( horz( s->direction() ) != horz( direction() ) )
	return FALSE;
    int n = chain.size();
    chain.resize( n + 1 );
    chain[n] = s;
    return TRUE;
}

QBasicManager::QBasicManager( QWidget *parent, const char *name )
    : QObject( parent, name )
{
    main = parent;

    xC = new ParChain( LeftToRight );
    yC = new ParChain(	Down );

    if ( parent )
	parent->installEventFilter( this );
}


QChain * QBasicManager::newParChain( Direction d )
{
    QChain * c = new ParChain( d );
    CHECK_PTR(c);
    return c;
}


QChain * QBasicManager::newSerChain( Direction d )
{
    QChain * c = new SerChain( d );
    CHECK_PTR(c);
    return c;
}


bool QBasicManager::add( QChain *destination, QChain *source, int stretch )
{
    return destination->add(source, stretch);
}

bool QBasicManager::addWidget( QChain *d, QWidget *w, int stretch )
{
    // int i, j;
    // if ( !w->minimumSize( &i, &j ) )
    //	w->setMinimumSize( w->width(), w->height() ); //######

    return d->add( new WidChain( d->direction(), w) , stretch );
}

bool QBasicManager::addSpacing( QChain *d, int minSize, int stretch, int maxSize )
{
    return d->add( new SpaceChain( d->direction(), minSize, maxSize), stretch  );
}

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


bool QBasicManager::doIt()
{
    yC->recalc();
    xC->recalc();

    int ys = yC->minSize() + 2*border;
    int xs = xC->minSize() + 2*border;

    if (  main->width() < xs && main->height() < ys )
	main->resize( xs , ys );
    else if (  main->width() < xs )
	main->resize( xs , main->height() );
    else if ( main->height() < ys )
	main->resize( main->width(), ys );

    main->setMinimumSize( xs, ys );

    ys = yC->maxSize() + 2*border;
    if ( ys > QBasicManager::unlimited )
	ys = QBasicManager::unlimited;
    xs = xC->maxSize() + 2*border;
    if ( xs > QBasicManager::unlimited )
	xs = QBasicManager::unlimited;

    if (  main->width() > xs && main->height() > ys )
	main->resize( xs , ys );
    else if (  main->width() > xs )
	main->resize( xs , main->height() );
    else if ( main->height() > ys )
	main->resize( main->width(), ys );

    main->setMaximumSize( xs, ys );


    resizeAll();

    return TRUE;

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
