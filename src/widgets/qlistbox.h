/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.h#5 $
**
** Definition of QListBox widget class
**
** Author  : Eirik Eng
** Created : 941121
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
***********************************************************************/

#ifndef QLISTBOX_H
#define QLISTBOX_H

#include "qtablew.h"


#define LBI_String	0x8000
#define LBI_BitMap	2
#define LBI_UserDefined 1000

class QBitMap;

struct QLBItem {				// list box item
    QLBItem(){}
    QLBItem( QBitMap	*bm )  { bitmap=bm; type=LBI_BitMap; }
    QLBItem( const char *s  )  { string=s;  type=LBI_String; }
    int type;	 
    union {
	QBitMap	   *bitmap;
	const char *string;
	void	   *data;
    };
};


class QStrList;
class QLBItemList;

class QListBox : public QTableWidget		// list box class
{
    Q_OBJECT
public:
    QListBox( QWidget *parent=0, const char *name=0 );
   ~QListBox();

    void	 setStrList( const QStrList * );
    void	 setStrList( const char **, int numStrings );

    void	 insertStrList( const QStrList *, int index=-1 );
    void	 insertStrList( const char**, int numStrings, int index=-1 );

    void	 insertItem( const char *string, int index=-1 );
    void	 insertItem( const QBitMap *bitmap, int index=-1 );
    void	 inSort( const char *string );
    void	 removeItem( int index );

    const char	*string( int index ) const;	// get string at index
    QBitMap	*bitmap( int index ) const;	// get bitmap at index

    void	 changeItem( const char *string, int index );
    void	 changeItem( const QBitMap *bitmap, int index );
    void	 clear();
    void	 setStringCopy( bool );
    bool	 stringCopy();

    void	 setAutoUpdate( bool );
    bool	 autoUpdate() const;

    int		 count() const;

    void	 setTopItem( int index );
    void	 setCurrentItem( int index );
    int		 topItem() const;
    int		 currentItem() const;

    void	 setDragSelect( bool );
    void	 setAutoScroll( bool );		// scroll on drag
    void	 setAutoScrollBar( bool );
    void	 setScrollBar( bool );

    bool	 dragSelect() const;	
    bool	 autoScroll() const;
    bool	 autoScrollBar() const;
    bool	 scrollBar() const;

    void	 centerCurrentItem();
    int		 numItemsVisible();

signals:
    void	 highlighted( int index );
    void	 selected( int index );

protected:    
    void	 setUserItems( bool );
    bool	 userItems();

    virtual QLBItem *newItem();
    virtual void     deleteItem( QLBItem * );

    virtual void paintItem( QPainter *, int index );
    void	 insertItem( const QLBItem*, int index=-1 );
    void	 inSort( const QLBItem * );
    void	 changeItem( const QLBItem*, int index );
    QLBItem	*item( int index ) const;
    bool	 itemVisible( int index );

    int		 cellHeight( long );
    int		 cellWidth( long );
    void	 paintCell( QPainter *, long row, long col );

    void	 mousePressEvent( QMouseEvent * );
    void	 mouseMoveEvent( QMouseEvent * );
    void	 mouseReleaseEvent( QMouseEvent * );
    void	 mouseDoubleClickEvent( QMouseEvent * );
    void	 resizeEvent( QResizeEvent * );
    void	 timerEvent( QTimerEvent * );
    void	 keyPressEvent( QKeyEvent *e );

    int		 findItem( int yPos ) const;
    int		 itemYPos( int index ) const;
    void	 updateItem( int index, bool clear = TRUE );
    void	 clearList();	 

private:
    QLBItem	*newAny( const char *, const QBitMap * );
    void	 insertAny( const char *, const QBitMap *, 
			    const QLBItem *, int );
    void	 changeAny( const char *, const QBitMap *, 
			    const QLBItem *, int );
    void	 updateNumRows();
    void	 init();

    uint   doDrag	  : 1;
    uint   doAutoScroll	  : 1;
    uint   isTiming	  : 1;
    uint   scrollDown	  : 1;
    uint   stringsOnly	  : 1;
    uint   copyStrings	  : 1;
    uint   multiSelect	  : 1;
    uint   ownerDrawn	  : 1;
    int	   current;
    
    QLBItemList *itemList;
};


inline bool QListBox::stringCopy()
{
    return copyStrings;
}

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

inline int QListBox::currentItem() const
{
    return current;
}

inline int QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}

inline int QListBox::itemYPos( int index ) const
{
    return rowYPos( index );
}

inline void QListBox::updateItem( int index, bool clear )
{
    updateCell( index, 0,  clear );
}

inline int QListBox::topItem() const
{
    return topCell();
}

inline void QListBox::setTopItem( int index )
{
    setTopCell( index );
}

inline void QListBox::setAutoUpdate( bool b )
{
    QTableWidget::setAutoUpdate( b );
}

inline bool QListBox::autoUpdate() const
{
    return QTableWidget::autoUpdate();
}

#endif // QLISTBOX_H
