/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qlistbox.h#13 $
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


#define LBI_String	1			// list box item types
#define LBI_Pixmap	2
#define LBI_UserDefined 1000

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


class QListBox : public QTableWidget		// list box widget
{
    Q_OBJECT
public:
    QListBox( QWidget *parent=0, const char *name=0 );
   ~QListBox();

    int		count() const;

    void	setStrList( const QStrList * );
    void	setStrList( const char **, int numStrings=-1 );

    void	insertStrList( const QStrList *, int index=-1 );
    void	insertStrList( const char**, int numStrings=-1, int index=-1 );

    void	insertItem( const char *string, int index=-1 );
    void	insertItem( const QPixmap *pixmap, int index=-1 );
    void	inSort( const char *string );

    void	removeItem( int index );
    void	clear();

    const char *string( int index ) const;
    QPixmap    *pixmap( int index ) const;
    void	changeItem( const char *string, int index );
    void	changeItem( const QPixmap *pixmap, int index );

    bool	stringCopy()	const;
    void	setStringCopy( bool );

    bool	autoUpdate()	const;
    void	setAutoUpdate( bool );

    int		numItemsVisible() const;

    int		currentItem()	const;
    void	setCurrentItem( int index );
    void	centerCurrentItem();
    int		topItem()	const;
    void	setTopItem( int index );

    bool	dragSelect()		const;
    void	setDragSelect( bool );
    bool	autoScroll()		const;
    void	setAutoScroll( bool );
    bool	autoScrollBar()		const;
    void	setAutoScrollBar( bool );
    bool	scrollBar()		const;
    void	setScrollBar( bool );
    bool	autoBottomScrollBar()	const;
    void	setAutoBottomScrollBar( bool );
    bool	bottomScrollBar()	const;
    void	setBottomScrollBar( bool );
    bool	smoothScrolling()	const;
    void	setSmoothScrolling( bool );

signals:
    void	highlighted( int index );
    void	selected( int index );

protected:
    virtual int itemWidth( QLBItem * );
    virtual int itemHeight( QLBItem * );

    void	setUserItems( bool );
    bool	userItems();

    virtual QLBItem *newItem();
    virtual void     deleteItem( QLBItem * );

    virtual void paintItem( QPainter *, int index );
    void	insertItem( const QLBItem*, int index=-1 );
//    void	inSort( const QLBItem * );
    void	changeItem( const QLBItem*, int index );
    QLBItem    *item( int index ) const;
    bool	itemVisible( int index );

    int		cellHeight( long );
    void	paintCell( QPainter *, long row, long col );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent *e );
    void	resizeEvent( QResizeEvent * );
    void	timerEvent( QTimerEvent * );

    int		findItem( int yPos ) const;
    bool	itemYPos( int index, int *yPos ) const;
    void	updateItem( int index, bool clear = TRUE );
    void	clearList();
    void	updateCellWidth();

private:
    QLBItem    *newAny( const char *, const QPixmap * );
    void	insertAny( const char *, const QPixmap *,
			   const QLBItem *, int, bool );
    void	changeAny( const char *, const QPixmap *,
			   const QLBItem *, int );
    void	updateNumRows( bool );
    int		internalItemWidth( const QLBItem *,
				   const QFontMetrics & ) const;

    uint	doDrag		: 1;
    uint	doAutoScroll	: 1;
    uint	isTiming	: 1;
    uint	scrollDown	: 1;
    uint	stringsOnly	: 1;
    uint	copyStrings	: 1;
    uint	multiSelect	: 1;
    uint	ownerDrawn	: 1;
    int		current;
    QLBItemList *itemList;
};


#endif // QLISTBOX_H
