/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopmenu.h#22 $
**
** Definition of QPopupMenu class
**
** Author  : Haavard Nord
** Created : 941128
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPOPMENU_H
#define QPOPMENU_H

#include "qtablevw.h"
#include "qmenudta.h"


class QPopupMenu : public QTableView, public QMenuData
{
friend class QMenuData;
friend class QMenuBar;
    Q_OBJECT
public:
    QPopupMenu( QWidget *parent=0, const char *name=0 );
   ~QPopupMenu();

    void	popup( const QPoint & pos, int indexAtPoint = 0 );// open popup
    void	updateItem( int id );

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

signals:
    void	activated( int itemId );
    void	highlighted( int itemId );
    void	activatedRedirect( int itemId );// to parent menu
    void	highlightedRedirect( int itemId );

protected:
    int		cellHeight( int );
    int		cellWidth( int );
    void	paintCell( QPainter *, int, int );

    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	timerEvent( QTimerEvent * );

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
    void	accelActivated( int itemId );
    void	accelDestroyed();

private:
    void	menuContentsChanged();
    void	menuStateChanged();
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    void	paintAll();
    void	actSig( int );
    void	hilitSig( int );
    void	setFirstItemActive();
    void	hideAllPopups();
    void	hidePopups();
    bool	tryMenuBar( QMouseEvent * );
    void	byeMenuBar();

    int		itemAtPos( const QPoint & );
    int		itemPos( int index );
    void	updateSize();
    void	updateAccel( QWidget * );
    void	enableAccel( bool );

    QMenuItem  *selfItem;
    QAccel     *autoaccel;
    bool	accelDisabled;
    int		popupActive;
    int		tabMark;

private:	//Disabled copy constructor and operator=
    QPopupMenu( const QPopupMenu & ) {}
    QPopupMenu &operator=( const QPopupMenu & ) { return *this; }
};


#endif // QPOPMENU_H
