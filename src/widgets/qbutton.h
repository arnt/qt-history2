/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#40 $
**
** Definition of QButton widget class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUTTON_H
#define QBUTTON_H

#include "qwidget.h"


class QButtonGroup;				// button group (qbttngrp.h)


class QButton : public QWidget			// button class
{
    Q_OBJECT
public:
    QButton( QWidget *parent=0, const char *name=0 );
   ~QButton();

    const char *text()		const	{ return btext; }
    void	setText( const char * );
    const QPixmap *pixmap()	const	{ return bpixmap; }
    void	setPixmap( const QPixmap & );

    bool	isToggleButton() const	{ return toggleBt; }

    bool	isDown()	const	{ return buttonDown; }
    bool	isOn()		const	{ return buttonOn; }

    bool	autoResize()	const	{ return autoresize; }
    void	setAutoResize( bool );

signals:
    void	pressed();
    void	released();
    void	clicked();
    void	toggled( bool );

protected:
    void	setToggleButton( bool );
    void	setOn( bool );
    void	setDown( bool );

    virtual bool hitButton( const QPoint &pos ) const;
    virtual void drawButton( QPainter * );
    virtual void drawButtonLabel( QPainter * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	paintEvent( QPaintEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );

private:
    QString	btext;
    QPixmap    *bpixmap;
    uint	toggleBt	: 1;
    uint	buttonDown	: 1;
    uint	buttonOn	: 1;
    uint	mlbDown		: 1;
    uint	autoresize	: 1;
    QButtonGroup *group;

    friend class QButtonGroup;

private:	// Disabled copy constructor and operator=
    QButton( const QButton & ) {}
    QButton &operator=( const QButton & ) { return *this; }
};


#endif // QBUTTON_H
