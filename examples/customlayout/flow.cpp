/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "flow.h"

SimpleFlow::~SimpleFlow()
{
    deleteAllItems();
}

int SimpleFlow::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
	//Not all C++ compilers support "mutable" yet:
	SimpleFlow * mthis = (SimpleFlow*)this;
	int h = mthis->doLayout( QRect(0,0,w,0), TRUE );
	mthis->cached_hfw = h;
	mthis->cached_width = w;
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


void SimpleFlow::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    doLayout( r );
}

QLayoutItem *SimpleFlow::itemAt(int idx) const
{
    return list.value(idx);
}

QLayoutItem *SimpleFlow::takeAt(int idx)
{
    return idx >= 0 && idx < list.size() ? list.takeAt(idx) : 0;
}

int SimpleFlow::doLayout( const QRect &r, bool testonly )
{
    int x = r.x();
    int y = r.y();
    int h = 0;          //height of this line so far.
    int i = 0;
    while (i < list.size()) {
        QLayoutItem *o = list.at(i);
        ++i;
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
        h = qMax( h,  o->sizeHint().height() );
    }
    return y + h - r.y();
}

QSize SimpleFlow::minimumSize() const
{
    QSize s(0,0);
    int i = 0;
    while (i < list.size()) {
        QLayoutItem *o = list.at(i);
        ++i;
        s = s.expandedTo( o->minimumSize() );
    }
    return s;
}
