/****************************************************************************
**
** Definition of QScrollBar class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

class QTimer;

#ifndef QT_H
#include "qwidget.h"
#include "qrangecontrol.h"
#endif // QT_H

#ifndef QT_NO_SCROLLBAR

class Q_GUI_EXPORT QScrollBar : public QWidget, public QRangeControl
{
    Q_OBJECT
    Q_PROPERTY( int minValue READ minValue WRITE setMinValue )
    Q_PROPERTY( int maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( int lineStep READ lineStep WRITE setLineStep )
    Q_PROPERTY( int pageStep READ pageStep WRITE setPageStep )
    Q_PROPERTY( int value READ value WRITE setValue )
    Q_PROPERTY( bool tracking READ tracking WRITE setTracking )
    Q_PROPERTY( bool draggingSlider READ draggingSlider )
    Q_PROPERTY( Orientation orientation READ orientation WRITE setOrientation )

public:
    QScrollBar( QWidget *parent=0, const char* name = 0 );
    QScrollBar( Orientation, QWidget *parent=0, const char* name = 0 );
    QScrollBar( int minValue, int maxValue, int lineStep, int pageStep,
		int value, Orientation, QWidget *parent=0, const char* name = 0 );
    ~QScrollBar();

    virtual void setOrientation( Orientation );
    Orientation orientation() const;
    virtual void setTracking( bool enable );
    bool	tracking() const;
    bool	draggingSlider() const;

    virtual void setPalette( const QPalette & );
    virtual QSize sizeHint() const;
    virtual void setSizePolicy( QSizePolicy sp );
    void setSizePolicy( QSizePolicy::SizeType hor, QSizePolicy::SizeType ver, bool hfw = FALSE );

    int	 minValue() const;
    int	 maxValue() const;
    void setMinValue( int );
    void setMaxValue( int );
    int	 lineStep() const;
    int	 pageStep() const;
    void setLineStep( int );
    void setPageStep( int );
    int  value() const;

    int		sliderStart() const;
    QRect	sliderRect() const;

public slots:
    void setValue( int );

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
#ifndef QT_NO_WHEELEVENT
    void 	wheelEvent( QWheelEvent * );
#endif
    void	keyPressEvent( QKeyEvent * );
    void	resizeEvent( QResizeEvent * );
    void	paintEvent( QPaintEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	hideEvent( QHideEvent* );

    void	valueChange();
    void	stepChange();
    void	rangeChange();

    void        changeEvent( QEvent * );

private slots:
    void doAutoRepeat();

private:
    void init();
    void positionSliderFromValue();
    int calculateValueFromSlider() const;

    void startAutoRepeat();
    void stopAutoRepeat();

    int rangeValueToSliderPos( int val ) const;
    int sliderPosToRangeValue( int val ) const;

    void action( int control );

    void drawControls( uint controls, uint activeControl ) const;
    void drawControls( uint controls, uint activeControl,
		       QPainter *p ) const;

    uint pressedControl;
    bool track;
    bool clickedAt;
    Orientation orient;

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
    return orient;
}

inline int QScrollBar::sliderStart() const
{
    return sliderPos;
}

inline void QScrollBar::setSizePolicy( QSizePolicy::SizeType hor, QSizePolicy::SizeType ver, bool hfw )
{
    QWidget::setSizePolicy( hor, ver, hfw );
}


#endif // QT_NO_SCROLLBAR

#endif // QSCROLLBAR_H
