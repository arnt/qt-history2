/****************************************************************************
** $Id: //depot/qt/main/examples/customlayout/border.h#1 $
**
** Definition of simple flow layout for custom layout example
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
****************************************************************************/

#ifndef BORDER_H
#define BORDER_H

#include <qlayout.h>
#include <qlist.h>

class BorderWidgetItem : public QWidgetItem
{
public:
    BorderWidgetItem( QWidget *w )
        : QWidgetItem( w )
    {}

    void setGeometry( const QRect &r )
    { widget()->setGeometry( r ); }

};

class BorderLayout : public QLayout
{
public:
    enum Position {
        West = 0,
        North,
        South,
        East,
        Center
    };

    struct BorderLayoutStruct
    {
        BorderLayoutStruct( QLayoutItem *i, Position p ) {
            item = i;
            pos = p;
        }

        QLayoutItem *item;
        Position pos;
    };

    enum SizeType {
        Minimum = 0,
        SizeHint
    };

    BorderLayout( QWidget *parent, int border = 0, int autoBorder = -1,
                  const char *name = 0 )
        : QLayout( parent, border, autoBorder, name ), cached( 0, 0 ), mcached( 0, 0 ),
          sizeDirty( TRUE ), msizeDirty( TRUE )
    {}

    BorderLayout( QLayout* parent, int autoBorder = -1, const char *name = 0 )
        : QLayout( parent, autoBorder, name  ), cached( 0, 0 ), mcached( 0, 0 ),
          sizeDirty( TRUE ), msizeDirty( TRUE )
    {}

    BorderLayout( int autoBorder = -1, const char *name = 0 )
        : QLayout( autoBorder, name ), cached( 0, 0 ), mcached( 0, 0 ),
          sizeDirty( TRUE ), msizeDirty( TRUE )
    {}

    void addItem( QLayoutItem *item )
    { add( item, West ); }

    void addWidget( QWidget *widget, Position pos )
    { add( new BorderWidgetItem( widget ), pos ); }

    void add( QLayoutItem *item, Position pos ) { 
        list.append( new BorderLayoutStruct( item, pos ) ); 
        sizeDirty = TRUE; msizeDirty = TRUE; 
        calcSize( SizeHint ); calcSize( Minimum );
    }

    bool hasHeightForWidth() const { return FALSE; }

    QSize sizeHint() const
    { return cached; }
    QSize minimumSize() const
    { return cached; }

    QLayoutIterator iterator();

    QSizePolicy::ExpandData expanding() const
    { return QSizePolicy::BothDirections; }

protected:
    void setGeometry( const QRect &rect );

private:
    void layout( const QRect &rect, bool testonly = FALSE );
    void calcSize( SizeType st );

    QList<BorderLayoutStruct> list;
    QSize cached, mcached;
    bool sizeDirty, msizeDirty;

};

#endif
