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

#include "border.h"

QLayoutItem *BorderLayout::itemAt(int idx)
{
    BorderLayoutStruct *b =  list.value(idx);
    return b ? b->item : 0;
}

QLayoutItem *BorderLayout::takeAt(int idx)
{
    if (idx < 0 || idx >= list.count())
        return 0;
    BorderLayoutStruct *b =  list.takeAt(idx);
    QLayoutItem *item = b->item;
    delete b;
    return item;
}


BorderLayout::~BorderLayout()
{
    deleteAllItems();
}


void BorderLayout::addItem( QLayoutItem *item )
{
    add( item, West );
}

void BorderLayout::addWidget( QWidget *widget, Position pos )
{
    add( new BorderWidgetItem( widget ), pos );
    addChildWidget( widget );
}

void BorderLayout::add( QLayoutItem *item, Position pos )
{
    list.append( new BorderLayoutStruct( item, pos ) );
    calcSize();
}

bool BorderLayout::hasHeightForWidth() const
{
    return FALSE;
}

QSize BorderLayout::sizeHint() const
{
    return cached;
}

QSize BorderLayout::minimumSize() const
{
    return mcached;
}

QSizePolicy::ExpandData BorderLayout::expanding() const

{
    return QSizePolicy::BothDirections;
}

void BorderLayout::setGeometry( const QRect &rct )
{
    QLayout::setGeometry( rct );
    doLayout( rct );
}

void BorderLayout::doLayout( const QRect &rct )
{
    int ew = 0, ww = 0, nh = 0, sh = 0;
    int h = 0;

    int y0 = rct.y();
    int x0 = rct.x();

    BorderLayoutStruct *center = 0;
    int i = 0;
    while (i < list.size()) {
        BorderLayoutStruct *o = list.at(i);
        ++i;

        if ( o->pos == North ) {
            o->item->setGeometry( QRect( x0, y0+nh, rct.width(), o->item->sizeHint().height() ) );
            nh += o->item->geometry().height() + spacing();
        }
        if ( o->pos == South ) {
            o->item->setGeometry( QRect( o->item->geometry().x(), o->item->geometry().y(),
                                         rct.width(), o->item->sizeHint().height() ) );
            sh += o->item->geometry().height() + spacing();
            o->item->setGeometry( QRect( x0, y0 + rct.height() - sh + spacing(),
                                         o->item->geometry().width(), o->item->geometry().height() ) );
        }
        if ( o->pos == Center )
            center = o;
    }

    h = rct.height() - nh - sh;

    i = 0;
    while (i < list.size()) {
        BorderLayoutStruct *o = list.at(i);
        ++i;

        if ( o->pos == West ) {
            o->item->setGeometry( QRect( x0 + ww, y0+nh, o->item->sizeHint().width(), h ) );
            ww += o->item->geometry().width() + spacing();
        }
        if ( o->pos == East ) {
            o->item->setGeometry( QRect( o->item->geometry().x(), o->item->geometry().y(),
                                         o->item->sizeHint().width(), h ) );
            ew += o->item->geometry().width() + spacing();
            o->item->setGeometry( QRect( x0 + rct.width() - ew + spacing(), y0+nh,
                                         o->item->geometry().width(), o->item->geometry().height() ) );
        }
    }

    if ( center )
        center->item->setGeometry( QRect( x0+ww, y0+nh, rct.width() - ew - ww, h ) );
}

void BorderLayout::calcSize()
{
    int mw = 0, mh = 0;
    int sw = 0, sh = 0;

    int i = 0;
    while (i < list.size()) {
        BorderLayoutStruct *o = list.at(i);
        ++i;
        if ( o->pos == North ||
             o->pos == South ) {
            mh += o->item->minimumSize().height();
            sh += o->item->sizeHint().height();
        }
        else if ( o->pos == West ||
                  o->pos == East ) {
            mw += o->item->minimumSize().width();
            sw += o->item->sizeHint().width();
        } else {
            mh += o->item->minimumSize().height();
            mw += o->item->minimumSize().width();
            sh += o->item->sizeHint().height();
            sw += o->item->sizeHint().width();
        }
    }

    mcached = QSize( mw, mh );
    cached = QSize( sw, sh );
}
