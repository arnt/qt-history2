/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qbutton.h#74 $
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
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


class QButtonGroup;
struct QButtonData;


class Q_EXPORT QButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString, "text", text, setText )
    Q_PROPERTY( QPixmap, "pixmap", pixmap, setPixmap )
    Q_PROPERTY( int, "accel", accel, setAccel )
    Q_PROPERTY( bool, "toggleButton", isToggleButton, 0 )
    Q_PROPERTY( ToggleType, "toggleType", toggleType, 0 )
    Q_PROPERTY( bool, "down", isDown, setDown )
    Q_PROPERTY( bool, "on", isOn, 0 )
    Q_PROPERTY( ToggleState, "toggleState", state, 0 )
    Q_PROPERTY( bool, "autoResize", autoResize, setAutoResize )
    Q_PROPERTY( bool, "autoRepeat", autoRepeat, setAutoRepeat )
    Q_PROPERTY( bool, "exclusiveToggle", isExclusiveToggle, 0 )
public:
    QButton( QWidget *parent=0, const char *name=0, WFlags f=0 );
   ~QButton();

    QString text() const;
    virtual void setText( const QString &);
    const QPixmap *pixmap() const;
    virtual void setPixmap( const QPixmap & );

    int		accel()	const;
    virtual void	setAccel( int );

    bool	isToggleButton() const;

    enum ToggleType { SingleShot, Toggle, Tristate };
    ToggleType	toggleType() const;

    virtual void setDown( bool );
    bool	isDown() const;

    bool	isOn() const;

    enum ToggleState { Off, NoChange, On };
    ToggleState	state() const;

    bool	autoResize() const; // obsolete
    virtual void setAutoResize( bool ); // obsolete

    bool	autoRepeat() const;
    virtual void setAutoRepeat( bool );

    bool	isExclusiveToggle() const;

    bool	focusNextPrevChild( bool next );

    QButtonGroup *group() const;

public slots:
    void	animateClick();
    void	toggle();

signals:
    void	pressed();
    void	released();
    void	clicked();
    void	toggled( bool );
    void	stateChanged( int );

protected:
    void	setToggleButton( bool );
    virtual void	setToggleType( ToggleType );
    void	setOn( bool );
    virtual void	setState( ToggleState );

    virtual bool hitButton( const QPoint &pos ) const;
    virtual void drawButton( QPainter * );
    virtual void drawButtonLabel( QPainter * );

    void	keyPressEvent( QKeyEvent *);
    void	keyReleaseEvent( QKeyEvent *);
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
    uint	toggleTyp	: 2;
    uint	buttonDown	: 1;
    uint	stat		: 2;
    uint	mlbDown		: 1;
    uint	autoresize	: 1;
    uint	animation	: 1;
    uint	repeat		: 1;
    QButtonData *d;

    friend class QButtonGroup;
    void          ensureData();
    virtual void setGroup( QButtonGroup* );
    QTimer	 *timer();
    void	nextState();

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
    return toggleTyp != SingleShot;
}

inline  bool QButton::isDown() const
{
    return buttonDown;
}

inline bool QButton::isOn() const
{
    return stat != Off;
}

inline bool QButton::autoResize() const // obsolete
{
    return autoresize;
}

inline bool QButton::autoRepeat() const
{
    return repeat;
}

inline QButton::ToggleState QButton::state() const
{
    return ToggleState(stat);
}

inline void QButton::setToggleButton( bool b )
{
    setToggleType( b ? Toggle : SingleShot );
}

inline void QButton::setOn( bool y )
{
    setState( y ? On : Off );
}

inline QButton::ToggleType QButton::toggleType() const
{
    return ToggleType(toggleTyp);
}


#endif // QBUTTON_H
