/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.h#10 $
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


#define LBI_String      1
#define LBI_Pixmap	2
#define LBI_UserDefined 1000

class QPixmap;

struct QLBItem {				// list box item
    QLBItem(){}
    QLBItem( QPixmap	*bm )  { pixmap=bm; type=LBI_Pixmap; }
    QLBItem( const char *s  )  { string=s;  type=LBI_String; }
    int type;
    union {
	QPixmap	   *pixmap;
	const char *string;
	void	   *data;
    };
};

class QStrList;
class QLBItemList;
class QFontMetrics;

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
    void	 insertItem( const QPixmap *pixmap, int index=-1 );
    void	 inSort( const char *string );
    void	 removeItem( int index );

    const char	*string( int index ) const;	// get string at index
    QPixmap	*pixmap( int index ) const;	// get pixmap at index

    void	 changeItem( const char *string, int index );
    void	 changeItem( const QPixmap *pixmap, int index );
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
    void         setAutoBottomScrollBar( bool );
    void         setBottomScrollBar( bool );
    void         setSmoothScrolling( bool );

    bool	 dragSelect() const;	
    bool	 autoScroll() const;
    bool	 autoScrollBar() const;
    bool         autoBottomScrollBar() const;
    bool         bottomScrollBar() const;
    bool	 scrollBar() const;
    bool         smoothScrolling() const;

    void	 centerCurrentItem();
    int		 numItemsVisible();

signals:
    void	 highlighted( int index );
    void	 selected( int index );

protected:    
    virtual int itemWidth( QLBItem * );
    virtual int itemHeight( QLBItem * );

    void	 setUserItems( bool );
    bool	 userItems();

    virtual QLBItem *newItem();
    virtual void     deleteItem( QLBItem * );

    virtual void paintItem( QPainter *, int index );
    void	 insertItem( const QLBItem*, int index=-1 );
//    void	 inSort( const QLBItem * );
    void	 changeItem( const QLBItem*, int index );
    QLBItem	*item( int index ) const;
    bool	 itemVisible( int index );

    int		 cellHeight( long );
    int		 cellWidth( long );
    int		 cellHeight(){return QTableWidget::cellHeight();} //why,
    int		 cellWidth(){return QTableWidget::cellHeight();} //Bjarne, why?
    void	 paintCell( QPainter *, long row, long col );

    void	 mousePressEvent( QMouseEvent * );
    void	 mouseMoveEvent( QMouseEvent * );
    void	 mouseReleaseEvent( QMouseEvent * );
    void	 mouseDoubleClickEvent( QMouseEvent * );
    void	 resizeEvent( QResizeEvent * );
    void	 timerEvent( QTimerEvent * );
    void	 keyPressEvent( QKeyEvent *e );
    void         setFont( QFont &f );

    int		 findItem( int yPos ) const;
    bool	 itemYPos( int index, int *yPos ) const;
    void	 updateItem( int index, bool clear = TRUE );
    void	 clearList();	 
    void         updateCellWidth();
private:
    QLBItem	*newAny( const char *, const QPixmap * );
    void	 insertAny( const char *, const QPixmap *, 
			    const QLBItem *, int,
                            bool updateCellWidth = TRUE );
    void	 changeAny( const char *, const QPixmap *, 
			    const QLBItem *, int );
    void	 updateNumRows( bool updateWidth = TRUE );
    int          internalItemWidth( const QLBItem *,
                                    const QFontMetrics & ) const;
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
    return testFlag( Tbl_autoVScrollBar );
}

inline void QListBox::setAutoScrollBar( bool b )
{
    setFlag( Tbl_autoVScrollBar );
}

inline bool QListBox::scrollBar() const
{
    return testFlag( Tbl_vScrollBar );
}

inline void QListBox::setScrollBar( bool b )
{
    setFlag( Tbl_vScrollBar );
}

inline bool QListBox::autoBottomScrollBar() const
{
    return testFlag( Tbl_autoHScrollBar );
}

inline void QListBox::setAutoBottomScrollBar( bool b )
{
    setFlag( Tbl_autoHScrollBar );
}

inline bool QListBox::bottomScrollBar() const
{
    return testFlag( Tbl_hScrollBar );
}

inline void QListBox::setBottomScrollBar( bool b )
{
    setFlag( Tbl_hScrollBar );
}

inline bool QListBox::smoothScrolling() const
{
    return testFlag( Tbl_smoothVScrolling );
}

inline void QListBox::setSmoothScrolling( bool b )
{
    setFlag( Tbl_smoothVScrolling );
}

inline int QListBox::currentItem() const
{
    return current;
}

inline int QListBox::findItem( int yPos ) const
{
    return findRow( yPos );
}

inline bool QListBox::itemYPos( int index, int *yPos ) const
{
    
    return rowYPos( index, yPos );
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
