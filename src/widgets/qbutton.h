/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#68 $
**
** Definition of QButton widget class
**
** Created : 940206
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

#ifndef QBUTTON_H
#define QBUTTON_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H


class QButtonGroup;				// button group (qbuttongroup.h)
struct QButtonData;


class Q_EXPORT QButton : public QWidget			// button class
{
    Q_OBJECT
public:
    QButton( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QButton();

    QString text() const;
    virtual void	setText( const QString &);
    const QPixmap *pixmap() const;
    virtual void	setPixmap( const QPixmap & );

    int		accel()	const;
    virtual void	setAccel( int );

    bool	isToggleButton() const;

    virtual void	setDown( bool );
    bool	isDown() const;

    bool	isOn() const;

    bool	autoResize() const;
    virtual void	setAutoResize( bool );

    bool	autoRepeat() const;
    virtual void	setAutoRepeat( bool );

public slots:
    void	animateClick();
    void	toggle();

signals:
    void	pressed();
    void	released();
    void	clicked();
    void	toggled( bool );

protected:
    virtual void	setToggleButton( bool );
    virtual void	setOn( bool );

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
    friend class QWidgetStack; // ### group() is private - why?
    void          ensureData();
    QButtonGroup *group() const;
    virtual void	  setGroup( QButtonGroup* );
    QTimer	 *timer();
private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QButton( const QButton & );
    QButton &operator=( const QButton & );
#endif
};


inline QString QButton::text() const
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
