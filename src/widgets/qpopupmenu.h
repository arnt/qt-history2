/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.h#1 $
**
** Definition of QPopupMenu class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPOPMENU_H
#define QPOPMENU_H

#include "qtablew.h"


#if defined(QPOPMENU_C) || defined(QMENUBAR_C)

class QPopupMenu;

class QMenuItem					// internal menu item class
{
friend class QPopupMenu;
friend class QMenuBar;
public:
    QMenuItem();
   ~QMenuItem();
private:
    int		id;				// item identifier
    QString	text;				// item text
    QBitMap    *bitmap;				// item bitmap (text ignored)
    QPopupMenu *submenu;			// item submenu
    QSignal    *signal;
    uint	isSeparator : 1;		// is item a separator?
    uint	isDisabled  : 1;		// is item disabled?
    uint	isChecked   : 1;		// is item checked?
};

#include "qlist.h"
typedef declare(QListM,QMenuItem)	  QMenuItemList;
typedef declare(QListIteratorM,QMenuItem) QMenuItemListIt;

#else

class QMenuItemList;

#endif


class QPopupMenu : public QTableWidget		// popup menu widget
{
    Q_OBJECT
public:
    QPopupMenu( QWidget *parent=0, const char *name=0 );

    void	popup( const QPoint & pos, int item=-1 );

    void	insertItem( const char *text, int id=-1, int index=-1 );
    void	insertItem( const char *text, QPopupMenu *subMenu,
			    int index=-1 );
    void	insertItem( QBitMap *bitmap, int id=-1, int index=-1 );
    void	insertItem( QBitMap *bitmap, QPopupMenu *subMenu,
			    int index=-1 );
    void	insertSeparator( int index=-1 );
    void	removeItem( int index );

    int		index( int id ) const;		// get index of specified item

    bool	isItemDisabled( int id ) const;
    bool	isItemEnabled( int id )	 const	{ return !isItemDisabled(id); }
    void	setItemEnabled( int id, bool onOff );
    void	enableItem( int id )		{ setItemEnabled( id, TRUE ); }
    void	disableItem( int id )		{ setItemEnabled( id, FALSE );}

    bool	connectItem( int id, const QObject *receiver,
			     const char *member );

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

signals:
    void	activated( int itemId );
    void	selected( int itemId );

protected:
    void	paintCell( QPainter *, long, long );
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	timerEvent( QTimerEvent * );

slots:
    void	subActivated( int itemId );
    void	subSelected( int itemId );

private:
    void	insertAny( const char *, QBitMap *, QPopupMenu *, int, int );
    void	hideAllMenus();
    void	hideSubMenus();
    int		itemAtPos( const QPoint &p );
    void	updateSize();
    int		cellHeight( long );
    int		cellWidth( long );
    QWidget    *popupParent;
    QMenuItemList *items;
    int		activeItem;
    short	cellh, cellw;
    QSize	knownSize;
    uint	isTopLevel	: 1;
    uint	firstMouseUp	: 1;
    uint	badSize	: 1;
};


#endif // QPOPMENU_H
