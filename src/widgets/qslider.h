/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.h#2 $
**
** Definition of QSlider class
**
** Created : 961020
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSLIDER_H
#define QSLIDER_H

#include "qwidget.h"
#include "qrangect.h"


class QSlider : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QSlider( QWidget *parent=0, const char *name=0 );
    QSlider( Orientation, QWidget *parent=0, const char *name=0 );
    QSlider( int minValue, int maxValue, int step, int value, Orientation,
		QWidget *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    virtual void setPalette( const QPalette & );

signals:
    void	valueChanged( int value );
    void	sliderPressed();
    void	sliderMoved( int value );
    void	sliderReleased();

protected:
    void	timerEvent( QTimerEvent * );

    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

    void	valueChange();
    void	rangeChange();

    QRect	sliderRect() const;

private:
    void	init();
    int		positionFromValue( int ) const;
    int		valueFromPosition( int ) const;
    void	paintSlider( int, int );
    bool	track;

    int		timerId;
    QCOORD	sliderPos;
    int		sliderVal;
    QCOORD	clickOffset;
    enum State { None, Dragging, TimingUp, TimingDown };
    State	state;
    Orientation orient;

private:	// Disabled copy constructor and operator=
    QSlider( const QSlider & ) {}
    QSlider &operator=( const QSlider & ) { return *this; }
};

inline void QSlider::setTracking( bool t )
{
    track = t;
}

inline bool QSlider::tracking() const
{
    return track;
}

inline QSlider::Orientation QSlider::orientation() const
{
    return orient;
}

#endif //QSLIDER_H
