/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.h#19 $
**
** Definition of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSCRBAR_H
#define QSCRBAR_H

#include "qwidget.h"
#include "qrangect.h"


class QScrollBar : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    enum Orientation { Horizontal, Vertical };

    QScrollBar( QWidget *parent=0, const char *name=0 );
    QScrollBar( Orientation, QWidget *parent=0, const char *name=0 );
    QScrollBar( long minValue, long maxValue, long LineStep, long PageStep,
		long value, Orientation,
		QWidget *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool enable );
    bool	tracking() const;

    virtual void setPalette( const QPalette & );

signals:
    void	valueChanged( long value );
    void	sliderPressed();
    void	sliderMoved( long value );
    void	sliderReleased();
    void	nextLine();
    void	previousLine();
    void	nextPage();
    void	previousPage();

protected:
    void	timerEvent( QTimerEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );

    void	valueChange();
    void	stepChange();
    void	rangeChange();

    int		sliderStart() const;
    QRect	sliderRect() const;

private:
    void	init();
    void	positionSliderFromValue();
    long	calculateValueFromSlider() const;

    uint	pressedControl	 : 8;
    uint	track		 : 1;
    uint	clickedAt	 : 1;
    uint	orient		 : 1;
    uint	thresholdReached : 1;
    uint	isTiming	 : 1;

    long	slidePreviousVal;
    QCOORD	sliderPos;
    QCOORD	clickOffset;
};


inline void QScrollBar::setTracking( bool t )
{
    track = t;
}

inline bool QScrollBar::tracking() const
{
    return track;
}

inline QScrollBar::Orientation QScrollBar::orientation() const
{
    return (Orientation)orient;
}

inline int QScrollBar::sliderStart() const
{
    return sliderPos;
}


enum ArrowType
    { UpArrow, DownArrow, LeftArrow, RightArrow };

void qDrawArrow( QPainter *, ArrowType type, GUIStyle style, bool down,
		 int x, int y, int w, int h, const QColorGroup & );


#endif // QSCRBAR_H
