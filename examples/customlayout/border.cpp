/****************************************************************************
** $Id: //depot/qt/main/examples/customlayout/border.cpp#1 $
**
** Implementing your own layout: flow example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "border.h"

class BorderLayoutIterator : public QGLayoutIterator
{
public:
    BorderLayoutIterator( const QList<BorderLayout::BorderLayoutStruct> *l )
        : idx( 0 ) , list( (QList<BorderLayout::BorderLayoutStruct>*)l )
    {}

    uint count() const
    { return list->count(); }

    QLayoutItem *current()
    { return idx < (int)count() ? list->at( idx )->item : 0; }

    BorderLayout::BorderLayoutStruct *currentStruct()
    { return idx < (int)count() ? list->at( idx ) : 0; }

    void toFirst()
    { idx = 0; }

    QLayoutItem *next()
    { idx++;  return current(); }

    void removeCurrent()
    { list->remove(  idx  ); }

    BorderLayoutIterator &operator++()
    { next();  return *this; }

private:
    int idx;
    QList<BorderLayout::BorderLayoutStruct> *list;

};

QLayoutIterator BorderLayout::iterator()
{
    return QLayoutIterator( new BorderLayoutIterator( &list ) );
}

void BorderLayout::setGeometry( const QRect &rect )
{
    QLayout::setGeometry( rect );
    layout( rect );
}

void BorderLayout::layout( const QRect &rect, bool /*testonly*/ )
{
    int ew = 0, ww = 0, nh = 0, sh = 0;
    int h = 0;

    BorderLayoutIterator it = BorderLayoutIterator( &list );
    BorderLayoutStruct *o;
    BorderLayoutStruct *center = 0L;
    while ( ( o = it.currentStruct() ) != 0 ) {
        ++it;

        if ( o->pos == North ) {
            o->item->setGeometry( QRect( rect.x(), nh, rect.width(), o->item->sizeHint().height() ) );
            nh += o->item->geometry().height() + spacing();
        }
        if ( o->pos == South ) {
            o->item->setGeometry( QRect( o->item->geometry().x(), o->item->geometry().y(),
                                         rect.width(), o->item->sizeHint().height() ) );
            sh += o->item->geometry().height() + spacing();
            o->item->setGeometry( QRect( rect.x(), rect.y() + rect.height() - sh + spacing(),
                                         o->item->geometry().width(), o->item->geometry().height() ) );
        }
        if ( o->pos == Center )
            center = o;
    }

    h = rect.height() - nh - sh;

    it.toFirst();
    while ( ( o = it.currentStruct() ) != 0 ) {
        ++it;

        if ( o->pos == West ) {
            o->item->setGeometry( QRect( rect.x() + ww, nh, o->item->sizeHint().width(), h ) );
            ww += o->item->geometry().width() + spacing();
        }
        if ( o->pos == East ) {
            o->item->setGeometry( QRect( o->item->geometry().x(), o->item->geometry().y(),
                                         o->item->sizeHint().width(), h ) );
            ew += o->item->geometry().width() + spacing();
            o->item->setGeometry( QRect( rect.x() + rect.width() - ew + spacing(), nh,
                                         o->item->geometry().width(), o->item->geometry().height() ) );
        }
    }

    if ( center )
        center->item->setGeometry( QRect( ww, nh, rect.width() - ew - ww, h ) );
}

void BorderLayout::calcSize( SizeType st )
{
    if ( ( st == Minimum && !msizeDirty ) ||
         ( st == SizeHint && !sizeDirty ) )
        return;

    int w = 0, h = 0;

    BorderLayoutIterator it = BorderLayoutIterator( &list );
    BorderLayoutStruct *o;
    while ( ( o = it.currentStruct() ) != 0 ) {
        ++it;
        if ( o->pos == North ||
             o->pos == South ) {
            if ( st == Minimum )
                h += o->item->minimumSize().height();
            else
                h += o->item->sizeHint().height();
        }
        else if ( o->pos == West ||
                  o->pos == East ) {
            if ( st == Minimum )
                w += o->item->minimumSize().width();
            else
                w += o->item->sizeHint().width();
        } else {
            if ( st == Minimum ) {
                h += o->item->minimumSize().height();
                w += o->item->minimumSize().width();
            }
            else {
                h += o->item->sizeHint().height();
                w += o->item->sizeHint().width();
            }
        }
    }

    if ( st == Minimum ) {
        msizeDirty = FALSE;
        mcached = QSize( w, h );
    } else {
        sizeDirty = FALSE;
        cached = QSize( w, h );
    }

    return;
}
