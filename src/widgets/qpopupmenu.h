/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.h#6 $
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
friend class QMenuBar;
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
    int		cellHeight( long );
    int		cellWidth( long );
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
    void	menuInsPopup( QPopupMenu * );	// menu popup item inserted
    void	menuDelPopup( QPopupMenu * );	// menu popup item deleted

    void	setFirstItemActive();
    void	hideAllPopups();
    void	hidePopups();
    bool	tryMenuBar( QMouseEvent * );
    void	byeMenuBar();

    int		itemAtPos( const QPoint & );
    void	updateSize();

    int		popupActive;
};


#endif // QPOPMENU_H
