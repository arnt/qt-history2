/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#20 $
**
** Definition of QButton widget class
**
** Author  : Haavard Nord
** Created : 940206
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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
    void	setText( const char *text );

    void	setAutoResizing( bool );
    bool	autoResizing()	const	{ return autoResize; }
    virtual void adjustSize();

signals:
    void	pressed();
    void	released();
    void	clicked();

protected:
    bool	isDown()	const	{ return buttonDown; }
    bool	isUp()		const	{ return !buttonDown; }

    bool	isOn()		const	{ return buttonOn; }
    void	switchOn();
    void	switchOff();

    bool	toggleButton()	const	{ return toggleBt; }
    void	setToggleButton( bool );

    virtual bool hitButton( const QPoint &pos ) const;
    virtual void drawButton( QPainter * );

    bool	pixmapUpdate()	const	{ return pmupdate; }
    void	setPixmapUpdate( bool enable );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	paintEvent( QPaintEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );

private:
    QString	btext;
    uint	toggleBt	: 1;
    uint	buttonDown	: 1;
    uint	buttonOn	: 1;
    uint	mlbDown		: 1;
    uint	autoResize	: 1;
    uint	pmupdate	: 1;
    QButtonGroup *group;

    friend class QButtonGroup;
};


#endif // QBUTTON_H
