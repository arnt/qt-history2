/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrbar.h#1 $
**
** Definition of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
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
    enum Direction{Horizontal,Vertical};

    QScrollBar(QView *parent = NULL, Direction d = Vertical);
    
    QScrollBar(int minValue, int maxValue, int LineStep, int PageStep,
               int value   , QView *parent = NULL, Direction d = Vertical);

    void setDirection(Direction);
    void setTracking(bool t);
    bool isTracking() const;

signals:
    void newValue(int value);
private:
    enum ScrollControl {ADD_LINE,ADD_PAGE,SUBTRACT_LINE,SUBTRACT_PAGE,
	                FIRST,LAST,SLIDER,NONE};
			
    void     initialize();
    void     valueChange();
    void     stepChange();
    void     rangeChange();

    void     timerEvent(QTimerEvent *e);
    bool     keyPressEvent(QKeyEvent *e);
    void     resizeEvent(QResizeEvent *e);
    void     paintEvent(QPaintEvent *e);

    void     mousePressEvent(QMouseEvent *e);
    void     mouseReleaseEvent(QMouseEvent *e);
    void     mouseMoveEvent(QMouseEvent *e);

    ScrollControl pointOver(const QPoint &p);

    int      border()         const;
    int      length()         const;
    int      controlWidth()   const;
    int      buttonLength()   const;
    int      addButtonStart() const;
    int      sliderMinPos()   const;
    int      sliderMaxPos()   const;
    int      sliderLength()   const;

    QRect    addButton()      const;
    QRect    subtractButton() const;
    QRect    slider()         const;

    int      rangeValueToSliderPos(int val) const;
    int      sliderPosToRangeValue(int val) const;
    void     positionSliderFromValue();
    int      calculateValueFromSlider() const;

    int      calculateSliderLength() const;
    QRect    calculateSliderRect() const;

    void     action(ScrollControl control);
    
    void     drawControl(ScrollControl control) const;
    void     drawControl(ScrollControl control,QPainter &p) const;
    void     moveSlider(int newSliderPos);
    void     moveSlider(const QRect &oldSlider,const QRect &newSlider);

    void     drawMotifControl(ScrollControl control,QPainter &p) const;
    void     drawWindowsControl(ScrollControl control,QPainter &p) const;
    void     drawMacControl(ScrollControl control,QPainter &p) const;
    void     drawNeXTControl(ScrollControl control,QPainter &p) const;

    void     moveMotifSlider(const QRect &oldSlider,const QRect &newSlider);

    int      track            : 1;
    bool     clickedAt        : 1;
    int      pressedControl   : 3;
    int      direction        : 1;
    int      thresholdReached : 1;

    int      sliderPos;
    int      clickOffset;

    int      timerID;
};

inline void QScrollBar::setTracking(bool t)
{
    track = t;
}
    
inline bool QScrollBar::isTracking() const
{
    return track;
}


#endif // QSCRBAR_H
