/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmenubar.h#20 $
**
** Definition of QMenuBar class
**
** Created : 941209
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include "qpopmenu.h"


class QMenuBar : public QFrame, public QMenuData
{
friend class QPopupMenu;
    Q_OBJECT
public:
    QMenuBar( QWidget *parent=0, const char *name=0 );
   ~QMenuBar();

    void	updateItem( int id );

    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

    bool	eventFilter( QObject *, QEvent * );

    int		heightForWidth(int) const;

    enum { Never=0, InWindowsStyle=1 };
    int		separatorUsage() const;
    void	setSeparatorUsage( int when );

signals:
    void	activated( int itemId );
    void	highlighted( int itemId );

protected:
    void	drawContents( QPainter * );
    void	fontChange( const QFont & );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	resizeEvent( QResizeEvent * );
    void	menuContentsChanged();
    void	menuStateChanged();

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
    void	accelActivated( int itemId );
    void	accelDestroyed();

private:
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    bool	tryMouseEvent( QPopupMenu *, QMouseEvent * );
    void	tryKeyEvent( QPopupMenu *, QKeyEvent * );
    void	goodbye();
    void	openActPopup();
    void	hidePopups();

    int		calculateRects( int max_width = -1 );
    int		itemAtPos( const QPoint & );
    QRect	itemRect( int item );

    QAccel     *autoaccel;
    QRect      *irects;

private:	// Disabled copy constructor and operator=
    QMenuBar( const QMenuBar & ) {}
    QMenuBar &operator=( const QMenuBar & ) { return *this; }
};


#endif // QMENUBAR_H
