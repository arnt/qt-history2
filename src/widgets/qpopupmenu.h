/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qpopupmenu.h#55 $
**
** Definition of QPopupMenu class
**
** Created : 941128
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPOPUPMENU_H
#define QPOPUPMENU_H

#ifndef QT_H
#include "qtableview.h"
#include "qmenudata.h"
#endif // QT_H


class Q_EXPORT QPopupMenu : public QTableView, public QMenuData
{
    Q_OBJECT
public:
    QPopupMenu( QWidget *parent=0, const char *name=0 );
   ~QPopupMenu();

    void	popup( const QPoint & pos, int indexAtPoint = 0 );// open popup
    void	updateItem( int id );

    virtual void	setCheckable( bool );
    bool	isCheckable() const;

    void	setFont( const QFont & );	// reimplemented set font
    void	show();				// reimplemented show
    void	hide();				// reimplemented hide

    int		exec();
    int 	exec( const QPoint & pos, int indexAtPoint = 0 );// modal popup

    virtual void	setActiveItem( int );

signals:
    void	activated( int itemId );
    void	highlighted( int itemId );
    void	activatedRedirect( int itemId );// to parent menu
    void	highlightedRedirect( int itemId );
    void	aboutToShow();

protected:
    int		cellHeight( int );
    int		cellWidth( int );
    void	paintCell( QPainter *, int, int );

    void closeEvent( QCloseEvent *e );
    void	paintEvent( QPaintEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	timerEvent( QTimerEvent * );
    void     styleChange( GUIStyle );

private slots:
    void	subActivated( int itemId );
    void	subHighlighted( int itemId );
    void	accelActivated( int itemId );
    void	accelDestroyed();
    void	modalActivation( int );

    void	subMenuTimer();

private:
    void	menuContentsChanged();
    void	menuStateChanged();
    void	menuInsPopup( QPopupMenu * );
    void	menuDelPopup( QPopupMenu * );
    void	frameChanged();

    void	paintAll();
    void	actSig( int );
    void	hilitSig( int );
    virtual void	setFirstItemActive();
    void	hideAllPopups();
    void	hidePopups();
    bool	tryMenuBar( QMouseEvent * );
    void	byeMenuBar();

    int		itemAtPos( const QPoint & );
    int		itemPos( int index );
    void	updateSize();
    void	updateRow( int row );
    void	updateAccel( QWidget * );
    void	enableAccel( bool );

    void	setTabMark( int );
    int		tabMark();
    void	setCheckableFlag( bool );

    int getWidthOfCheckCol() const;

    QString 	accelString( int k );

    QMenuItem  *selfItem;
    QAccel     *autoaccel;
    bool	accelDisabled;
    int		popupActive;
    int		tabCheck;
    bool	hasDoubleItem;
    int		maxPMWidth;

    friend class QMenuData;
    friend class QMenuBar;
    
    void connectModal(QPopupMenu* receiver, bool doConnect);

    int internalCellHeight( QMenuItem* );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPopupMenu( const QPopupMenu & );
    QPopupMenu &operator=( const QPopupMenu & );
#endif
};


#endif // QPOPUPMENU_H
