/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.h#1 $
**
** Definition of QMenuBar class
**
** Author  : Haavard Nord
** Created : 941209
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include "qpopmenu.h"


class QMenuBar : public QWidget, public QMenuData
{
    Q_OBJECT
public:
    QMenuBar( QWidget *parent=0, const char *name=0 );
   ~QMenuBar();

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

signals:
    void	activated( int itemId );
    void	selected( int itemId );

protected:
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
    bool	eventFilter( QObject *, QEvent * );

    void	hideAllMenus();
    void	hideSubMenus();

    void	updateSize();
    int		itemAtPos( const QPoint & );
    QRect      *itemRects();
    QRect	itemRect( int item );

    QWidget    *popupParent;			// logical parent
    short	actItem;			// active item
    short	mbflags;			// internal menu bar flags
};


#endif // QMENUBAR_H
