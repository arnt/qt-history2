/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.h#35 $
**
** Definition of QSlider class
**
** Created : 961019
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

#ifndef QSLIDER_H
#define QSLIDER_H

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H


class QTimer;
struct QSliderData;


class Q_EXPORT QSlider : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum TickSetting { NoMarks = 0, Above = 1, Left = Above,
		       Below = 2, Right = Below, Both = 3 };

    QSlider( QWidget *parent, const char *name=0 );
    QSlider( Orientation, QWidget *parent, const char *name=0 );
    QSlider( int minValue, int maxValue, int pageStep, int value, Orientation,
	     QWidget *parent, const char *name=0 );

    virtual void	setOrientation( Orientation );
    Orientation orientation() const;
    virtual void	setTracking( bool enable );
    bool	tracking() const;

    virtual void 	setPalette( const QPalette & );
    QRect	sliderRect() const;
    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setTickmarks( TickSetting );
    TickSetting tickmarks() const { return ticks; }

    virtual void setTickInterval( int );
    int 	tickInterval() const { return tickInt; }

public slots:
    virtual void	setValue( int );
    void	addStep();
    void	subtractStep();

signals:
    void	valueChanged( int value );
    void	sliderPressed();
    void	sliderMoved( int value );
    void	sliderReleased();

protected:
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	keyPressEvent( QKeyEvent * );
    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	wheelEvent( QWheelEvent * );
    void	focusInEvent( QFocusEvent *e );
    void	focusOutEvent( QFocusEvent *e );

    void updateMask();

    void	valueChange();
    void	rangeChange();

    virtual void paintSlider( QPainter *, const QColorGroup&, const QRect & );
    void	drawTicks( QPainter *, const QColorGroup&, int, int, int=1 ) const;

    void	drawWinGroove( QPainter *,  QCOORD );
    void	drawTicks( QPainter *,  int, int, int=1 ) const;

    virtual int	thickness() const;


private slots:
    void	repeatTimeout();

private:
    enum State { Idle, Dragging, TimingUp, TimingDown };

    void	init();
    int		positionFromValue( int ) const;
    int		valueFromPosition( int ) const;
    void	moveSlider( int );
    void	reallyMoveSlider( int );
    void	resetState();
    int		slideLength() const;
    int		available() const;
    int		goodPart( const QPoint& ) const;
    void	initTicks();

    QSliderData *extra;
    QTimer	*timer;
    QCOORD	sliderPos;
    int		sliderVal;
    QCOORD	clickOffset;
    State	state;
    bool	track;
    QCOORD	tickOffset;
    TickSetting	ticks;
    int		tickInt;
    Orientation orient;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSlider( const QSlider & );
    QSlider &operator=( const QSlider & );
#endif
};

inline bool QSlider::tracking() const
{
    return track;
}

inline QSlider::Orientation QSlider::orientation() const
{
    return orient;
}


#endif // QSLIDER_H
