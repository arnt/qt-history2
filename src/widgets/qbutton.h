/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#52 $
**
** Definition of QButton widget class
**
** Created : 940206
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QBUTTON_H
#define QBUTTON_H

#include "qwidget.h"


class QButtonGroup;				// button group (qbttngrp.h)
struct QButtonData;


class QButton : public QWidget			// button class
{
    Q_OBJECT
public:
    QButton( QWidget *parent=0, const char *name=0 );
   ~QButton();

    const char *text() const;
    void	setText( const char * );
    const QPixmap *pixmap() const;
    void	setPixmap( const QPixmap & );

    int		accel()	const;
    void	setAccel( int );

    bool	isToggleButton() const;

    bool	isDown() const;
    bool	isOn() const;

    bool	autoResize() const;
    void	setAutoResize( bool );

    bool	autoRepeat() const;
    void	setAutoRepeat( bool );

public slots:
    void	animateClick();

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

    void	keyPressEvent( QKeyEvent *);
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	paintEvent( QPaintEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );

    void	enabledChange( bool );

private slots:
    void	animateTimeout();
    void	autoRepeatTimeout();

private:
    QString	btext;
    QPixmap    *bpixmap;
    uint	toggleBt	: 1;
    uint	buttonDown	: 1;
    uint	buttonOn	: 1;
    uint	mlbDown		: 1;
    uint	autoresize	: 1;
    uint	animation	: 1;
    uint	repeat		: 1;
    QButtonData *d;

    friend class QButtonGroup;
    void          ensureData();
    QButtonGroup *group() const;
    void	  setGroup( QButtonGroup* );
    QTimer	 *timer();

private:	// Disabled copy constructor and operator=
    QButton( const QButton & ) {}
    QButton &operator=( const QButton & ) { return *this; }
};


inline const char *QButton::text() const
{
    return btext;
}

inline const QPixmap *QButton::pixmap() const
{
    return bpixmap;
}

inline bool QButton::isToggleButton() const
{
    return toggleBt;
}

inline  bool QButton::isDown() const
{
    return buttonDown;
}

inline bool QButton::isOn() const
{
    return buttonOn;
}

inline bool QButton::autoResize() const
{
    return autoresize;
}

inline bool QButton::autoRepeat() const
{
    return repeat;
}


#endif // QBUTTON_H
