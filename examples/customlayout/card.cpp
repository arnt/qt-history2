/****************************************************************************
** $Id: //depot/qt/main/examples/customlayout/card.cpp#1 $
**
** Implementing your own layout: flow example
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "card.h"

//Normally it's a very bad idea to inline virtual functions
class CardLayoutIterator :public QGLayoutIterator
{
public:
    CardLayoutIterator( QList<QLayoutItem> *l )
        : idx( 0 ), list( l )  {}

    QLayoutItem *current()
    { return idx < int( list->count() ) ? list->at( idx ) : 0;  }

    QLayoutItem *next()
    { idx++; return current(); }

    void removeCurrent()
    { list->remove(  idx  ); }

private:
    int idx;
    QList<QLayoutItem> *list;
};

QLayoutIterator CardLayout::iterator()
{
    return QLayoutIterator(  new CardLayoutIterator( &list )  );
}

CardLayout::~CardLayout()
{
    deleteAllItems();
}

void CardLayout::addItem(  QLayoutItem *item  )
{
    list.append( item );
}

void CardLayout::setGeometry( const QRect &rect )
{
    QLayout::setGeometry( rect );

    QListIterator<QLayoutItem> it( list );
    if ( it.count() == 0 )
        return;

    QLayoutItem *o;

    int i = 0;

    int w = rect.width() - ( list.count() - 1 ) * spacing();
    int h = rect.height() - ( list.count() - 1 ) * spacing();

    while ( ( o=it.current() ) != 0 ) {
        ++it;
        QRect geom( rect.x() + i * spacing(), rect.y() + i * spacing(),
                    w, h  );
        o->setGeometry(  geom  );
        ++i;
    }
}

QSize CardLayout::sizeHint() const
{
    QSize s(0,0);
    int n = list.count();
    if ( n > 0 )
        s = QSize(100,70); //start with a nice default size
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
        ++it;
        s = s.expandedTo( o->minimumSize() );
    }
    return s + n*QSize(spacing(),spacing());
}

QSize CardLayout::minimumSize() const
{
    QSize s(0,0);
    int n = list.count();
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
        ++it;
        s = s.expandedTo( o->minimumSize() );
    }
    return s + n*QSize(spacing(),spacing());
}

