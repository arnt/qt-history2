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

#include "card.h"

CardLayout::~CardLayout()
{
    deleteAllItems();
}

void CardLayout::addItem(  QLayoutItem *item  )
{
    list.append( item );
}

void CardLayout::setGeometry( const QRect &rct )
{
    QLayout::setGeometry( rct );

    if ( list.size() == 0 )
        return;

    int w = rct.width() - (list.count() - 1) * spacing();
    int h = rct.height() - (list.count() - 1) * spacing();
    int i = 0;
    while (i < list.size()) {
        QLayoutItem *o = list.at(i);
        QRect geom(rct.x() + i * spacing(), rct.y() + i * spacing(), w, h);
        o->setGeometry(geom);
        ++i;
    }
}

QSize CardLayout::sizeHint() const
{
    QSize s(0,0);
    int n = list.count();
    if ( n > 0 )
        s = QSize(100,70); //start with a nice default size
    int i = 0;
    while (i < n) {
        QLayoutItem *o = list.at(i);
        s = s.expandedTo(o->sizeHint());
        ++i;
    }
    return s + n*QSize(spacing(), spacing());
}

QSize CardLayout::minimumSize() const
{
    QSize s(0,0);
    int n = list.count();
    int i = 0;
    while (i < n) {
        QLayoutItem *o = list.at(i);
        s = s.expandedTo(o->minimumSize());
        ++i;
    }
    return s + n*QSize(spacing(), spacing());
}


QLayoutItem *CardLayout::itemAt(int idx)
{
    return list.value(idx);
}

QLayoutItem *CardLayout::takeAt(int idx)
{
    return idx >= 0 && idx < list.size() ? list.takeAt(idx) : 0;
}
