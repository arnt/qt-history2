/**********************************************************************
** $Id: gaschmuck!
**
** Definition of QListBox class
**
** Author  : Eirik Eng
** Created : 941121
**
** Copyright (c) 1994 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLISTBOX_H
#define QLISTBOX_H

#include "qtablew.h"
#include "qstrlist.h"

class QStrList;

class QListBox : public QTableWidget
{
    Q_OBJECT
public:
    QListBox( QWidget *parent=0, const char *name = 0 );
    ~QListBox(){}

    void setStrList( QStrList *, bool copy = TRUE );
    void setStrList( const char **, int numStrings, bool copy = TRUE );

    bool dragSelect() const;
    void setDragSelect( bool );
    
    bool autoScroll() const;
    void setAutoScroll( bool );

    bool autoScrollBar() const;
    void setAutoScrollBar( bool );

    bool scrollBar() const;
    void setScrollBar( bool );

    long selectedItem() const;
    void setSelectedItem( long index );

    long topItem() const;
    void setTopItem( long index );

signals:
    void clicked( long index );
    void doubleClicked( long index );

protected:    
    bool   itemVisible( long index );

    void   paintCell( QPainter *, long row, long col );

    void   mousePressEvent( QMouseEvent * );
    void   mouseMoveEvent( QMouseEvent * );
    void   mouseReleaseEvent( QMouseEvent * );
    void   mouseDoubleClickEvent( QMouseEvent * );
    void   resizeEvent( QResizeEvent * );
    bool   keyPressEvent( QKeyEvent * ){};
    void   timerEvent( QTimerEvent * );

    long   findItem( int yPos ) const;
    int    itemYPos( long index ) const;
    void   updateItem( long index, bool clear = TRUE );

private:

    uint   doDrag        : 1;
    uint   doAutoScroll  : 1;
    uint   isTiming      : 1;
    uint   scrollDown    : 1;
//    uint   stringsOnly   : 1;
    long   selected;

    QStrList *strList;

};


inline bool QListBox::dragSelect() const
{
    return doDrag;
}

inline void QListBox::setDragSelect( bool b )
{
    doDrag = b;
}

inline bool QListBox::autoScroll() const
{
    return doAutoScroll;
}

inline void QListBox::setAutoScroll( bool b )
{
    doAutoScroll = b;
}

inline bool QListBox::autoScrollBar() const
{
    return autoVerScrollBar();
}

inline void QListBox::setAutoScrollBar( bool b )
{
    setAutoVerScrollBar( b );
}

inline bool QListBox::scrollBar() const
{
    return verScrollBar();
}

inline void QListBox::setScrollBar( bool b )
{
    setVerScrollBar( b );
}

inline long QListBox::selectedItem() const
{
    return selected;
}

inline long QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}

inline int QListBox::itemYPos( long index ) const
{
    return rowYPos( index );
}

inline void QListBox::updateItem( long index, bool clear )
{
    updateCell( index, 0,  clear );
}

inline long QListBox::topItem() const
{
    return topCell();
}

inline void QListBox::setTopItem( long index )
{
    setTopCell( index );
}

#endif
