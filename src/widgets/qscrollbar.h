/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.h#3 $
**
** Definition of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
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
    enum Orientation{ Horizontal, Vertical };

    QScrollBar( QView *parent=0, const char *name=0 );
    QScrollBar( Orientation, QView *parent=0, const char *name=0 );
    QScrollBar( int minValue, int maxValue, int LineStep, int PageStep,
		int value, Orientation,
		QView *parent=0, const char *name=0 );

    void	setOrientation( Orientation );
    Orientation orientation() const;
    void	setTracking( bool t );
    bool	tracking() const;

signals:
    void    	newValue( int value );

protected:
    void     	timerEvent( QTimerEvent *e );
    bool     	keyPressEvent( QKeyEvent *e );
    void     	resizeEvent( QResizeEvent *e );
    void     	paintEvent( QPaintEvent *e );

    void     	mousePressEvent( QMouseEvent *e );
    void     	mouseReleaseEvent( QMouseEvent *e );
    void     	mouseMoveEvent( QMouseEvent *e );

    void     	valueChange();	       // virtual functions from QRangeControl
    void     	stepChange();
    void     	rangeChange();

    int	     	sliderStart() const;

private:
    void     	init();
    void     	positionSliderFromValue();
    int	     	calculateValueFromSlider() const;

    uint     	pressedControl   : 8;
    uint     	track	         : 1;
    uint     	clickedAt	 : 1;
    uint     	orient	         : 1;
    uint     	thresholdReached : 1;
    uint     	isTiming	 : 1;

    QCOOT    	sliderPos;
    QCOOT    	clickOffset;
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


#endif // QSCRBAR_H
