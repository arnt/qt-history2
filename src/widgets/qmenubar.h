/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.h#6 $
**
** Definition of QMenuBar class
**
** Author  : Haavard Nord
** Created : 941209
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include "qpopmenu.h"


class QMenuBar : public QWidget, public QMenuData
{
friend class QPopupMenu;
    Q_OBJECT
public:
    QMenuBar( QWidget *parent=0, const char *name=0 );
   ~QMenuBar();

    void	updateItem( int id );

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

signals:
    void	activated( int itemId );
    void	selected( int itemId );

protected:
    void	paintEvent( QPaintEvent * );
    void	resizeEvent( QResizeEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );

slots:
    void	subActivated( int itemId );
    void	subSelected( int itemId );

private:
    void	menuContentsChanged();		// menu item inserted/removed
    void	menuStateChanged();		// menu item state changed
    void	menuInsPopup( QPopupMenu * );	// menu popup item inserted
    void	menuDelPopup( QPopupMenu * );	// menu popup item deleted

    bool	eventFilter( QObject *, QEvent * );

    bool	tryMouseEvent( QPopupMenu *, QMouseEvent * );
    void	tryKeyEvent( QPopupMenu *, QKeyEvent * );
    void	goodbye();
    void	openActPopup();
    void	hidePopups();

    void	updateRects();
    int		itemAtPos( const QPoint & );
    QRect	itemRect( int item );

    QRect      *irects;
};


#endif // QMENUBAR_H
