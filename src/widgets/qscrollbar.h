/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.h#44 $
**
** Definition of QScrollBar class
**
** Created : 940427
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

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

class QTimer;

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qdrawutil.h"
#endif // QT_H


class Q_EXPORT QScrollBar : public QWidget, public QRangeControl
{
    Q_OBJECT
public:
    QScrollBar( QWidget *parent, const char *name=0 );
    QScrollBar( Orientation, QWidget *parent, const char *name=0 );
    QScrollBar( int minValue, int maxValue, int LineStep, int PageStep,
		int value, Orientation,
		QWidget *parent, const char *name=0 );

    virtual void setOrientation( Orientation );
    Orientation orientation() const;
    virtual void setTracking( bool enable );
    bool	tracking() const;

    bool	draggingSlider() const;

    virtual void setPalette( const QPalette & );
    QSize	sizeHint() const;
    QSizePolicy sizePolicy() const;

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
    void 	wheelEvent( QWheelEvent * );
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

private slots:
    void doAutoRepeat();

private:
    void init();
    void positionSliderFromValue();
    int calculateValueFromSlider() const;

    void sliderMinMax( int &, int & )		const;
    void metrics( int &, int &, int &, int& )	const;

    void startAutoRepeat();
    void stopAutoRepeat();

    QStyle::ScrollControl pointOver( const QPoint &p ) const;

    int rangeValueToSliderPos( int val ) const;
    int sliderPosToRangeValue( int  val ) const;

    void action( QStyle::ScrollControl control );

    void drawControls( uint controls, uint activeControl ) const;
    void drawControls( uint controls, uint activeControl,
				QPainter *p ) const;

    uint pressedControl	 : 8;
    uint track		 : 1;
    uint clickedAt	 : 1;
    uint orient		 : 1;

    int slidePrevVal;
    QCOORD sliderPos;
    QCOORD clickOffset;

    QTimer * repeater;
    void * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QScrollBar( const QScrollBar & );
    QScrollBar &operator=( const QScrollBar & );
#endif
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


#endif // QSCROLLBAR_H
