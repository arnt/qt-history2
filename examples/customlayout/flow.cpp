/****************************************************************************
** $Id: //depot/qt/main/examples/customlayout/flow.cpp#2 $
**
** Implementing your own layout: flow example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "flow.h"

class SimpleFlowIterator :public QGLayoutIterator
{
public:
    SimpleFlowIterator( QList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const;
    QLayoutItem *current();
    QLayoutItem *next();
    void removeCurrent();

private:
    int idx;
    QList<QLayoutItem> *list;

};

uint SimpleFlowIterator::count() const 
{ 
    return list->count(); 
}

QLayoutItem *SimpleFlowIterator::current() 
{ 
    return idx < (int)count() ? list->at(idx) : 0;  
}

QLayoutItem *SimpleFlowIterator::next() 
{ 
    idx++; return current(); 
}

void SimpleFlowIterator::removeCurrent() 
{ 
    list->remove( idx ); 
}



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

bool SimpleFlow::hasHeightForWidth() const 
{ 
    return TRUE; 
}

QSize SimpleFlow::sizeHint() const 
{ 
    return minimumSize(); 
}

QSizePolicy::ExpandData SimpleFlow::expanding() const
{ 
    return QSizePolicy::NoDirection; 
}

QLayoutIterator SimpleFlow::iterator()
{
    return QLayoutIterator( new SimpleFlowIterator( &list ) );
}

void SimpleFlow::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    layout( r );
}

int SimpleFlow::layout( const QRect &r, bool testonly )
{
    int x = r.x();
    int y = r.y();
    int h = 0;		//height of this line so far.
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
	++it;
	int nextX = x + o->sizeHint().width() + spacing();
	if ( nextX - spacing() > r.right() && h > 0 ) {
	    x = r.x();
	    y = y + h + spacing();
	    nextX = x + o->sizeHint().width() + spacing();
	    h = 0;
	}
	if ( !testonly )
	    o->setGeometry( QRect( QPoint( x, y ), o->sizeHint() ) );
	x = nextX;
	h = QMAX( h,  o->sizeHint().height() );
    }
    return y + h - r.y();
}

QSize SimpleFlow::minimumSize() const
{
    QSize s(0,0);
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
	++it;
	s = s.expandedTo( o->minimumSize() );
    }
    return s;
}
