/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrbar.h#21 $
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
    QScrollBar( int minValue, int maxValue, int LineStep, int PageStep,
		int value, Orientation,
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
    void	nextLine();
    void	prevLine();
    void	nextPage();
    void	prevPage();

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
    int		calculateValueFromSlider() const;

    uint	pressedControl	 : 8;
    uint	track		 : 1;
    uint	clickedAt	 : 1;
    uint	orient		 : 1;
    uint	thresholdReached : 1;
    uint	isTiming	 : 1;

    int		slidePrevVal;
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
