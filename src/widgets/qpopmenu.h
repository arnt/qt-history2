/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.h#3 $
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
#include "qmenudta.h"


class QPopupMenu : public QTableWidget, public QMenuData
{
    Q_OBJECT
public:
    QPopupMenu( QWidget *parent=0, const char *name=0 );

    void	popup( const QPoint & pos );	// open popup

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
    void	menuContentsChanged();		// menu item inserted/removed
    void	menuStateChanged();		// menu item state changed
    void	menuInitSubMenu( QPopupMenu * );// menu sub popup inserted

    void	hideAllMenus();
    void	hideSubMenus();

    int		itemAtPos( const QPoint & );
    void	updateSize();
    int		cellHeight( long );
    int		cellWidth( long );

    QWidget    *popupParent;			// logical parent
    short	actItem;			// active item
    short	pmflags;			// internal popup menu flags
};


#endif // QPOPMENU_H
