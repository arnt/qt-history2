/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qslider.h#18 $
**
** Definition of QSlider class
**
** Created : 961019
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSLIDER_H
#define QSLIDER_H

#include "qwidget.h"
#include "qrangect.h"


class QTimer;
struct QSliderData;


class QSlider : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };
    enum TickSetting { NoMarks = 0, Above = 1, Left = Above, 
		       Below = 2, Right = Below, Both = 3 };

    QSlider( QWidget *parent=0, const char *name=0 );
    QSlider( Orientation, QWidget *parent=0, const char *name=0 );
    QSlider( int minValue, int maxValue, int step, int value, Orientation,
	     QWidget *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    void 	setPalette( const QPalette & );
    QRect	sliderRect() const;
    QSize	sizeHint() const;

    virtual void setTickmarks( TickSetting );
    QSlider::TickSetting tickmarks() const { return ticks; }

    virtual void setTickInterval( int );
    int 	tickInterval() const { return tickInt; }

public slots:
    void	setValue( int );
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

    void	valueChange();
    void	rangeChange();

    virtual void paintSlider( QPainter *, const QRect & );
    void	drawWinGroove( QPainter *, QCOORD );
    void	drawTicks( QPainter *, int, int, int=1 ) const;

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
    QSlider( const QSlider & ) {}
    QSlider &operator=( const QSlider & ) { return *this; }
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
