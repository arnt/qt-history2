/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qscrollbar.cpp#1 $
**
** Implementation of QScrollBar class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qscrbar.h"
#include "qpainter.h"
#include "qpntarry.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qscrollbar.cpp#1 $";
#endif


const int thresholdTime = 500;
const int repeatTime    = 100;

#define REF(X) X = X

QScrollBar::QScrollBar(QView *parent,Direction d,WFlags f)
    : QWidget(parent,f)
{
    direction = d;
    initialize();
}

QScrollBar::QScrollBar(int minVal, int maxVal, int lineStep, int pageStep,
                       int value, QView *parent, Direction d, WFlags f)
   : QWidget(parent,f),QRangeControl(minVal, maxVal, lineStep, pageStep, value)
{
    direction = d;
    initialize();
}

void QScrollBar::initialize()
{
    track            = TRUE;
    sliderPos        = 0;
    pressedControl   = NONE;
    timerID          = 0;
    clickedAt        = FALSE;
    setForegroundColor(lightGray);
    setBackgroundColor(foregroundColor().dark(1.12));
}

void QScrollBar::valueChange()
{
    moveSlider(rangeValueToSliderPos(value()));
    emit newValue(value());
}

void QScrollBar::stepChange()
{
    rangeChange();
}

void QScrollBar::rangeChange()
{
    QRect oldSlider = slider();

    sliderPos    = rangeValueToSliderPos(value());
    
    QPainter p;
    
    QBrush   b(foregroundColor().dark(1.12));
    QPen     pen;

    p  .begin(this);
    p  .setBrush(b);
    p  .setPen(pen);
    pen.setStyle(NoPen);
   
    p.drawRect(oldSlider);    
    drawControl(SLIDER,p);
    
    p.end();
	
}

void QScrollBar::timerEvent(QTimerEvent *)
{
    if (timerID == 0)
        return;
    if (!thresholdReached) {
	thresholdReached = TRUE;
	qKillTimer(timerID);	// hanord!!! QObject::killTimer
	timerID = qStartTimer(repeatTime,this);  // hanord!!! startTimer
    }
    if (clickedAt)
	action((ScrollControl) pressedControl);
}

bool QScrollBar::keyPressEvent(QKeyEvent *e)
{
    e = e;
    return TRUE;
}

void QScrollBar::resizeEvent(QResizeEvent *)
{
    sliderPos    = rangeValueToSliderPos(value());
}

void QScrollBar::paintEvent(QPaintEvent *)
{
    
    QPainter p;
    QPen     pen;
    
    p.begin(this);
    p.setBackgroundColor(foregroundColor());

    p.setPen(pen);

    p.drawShadePanel(clientRect(),foregroundColor().dark(),
                                  foregroundColor().light(),2,2);
    drawControl(ADD_LINE,p);
    drawControl(SUBTRACT_LINE,p);
    drawControl(SLIDER,p);
    p.end();
}

void QScrollBar::mousePressEvent(QMouseEvent *e)
{
    clickedAt      = TRUE;
    pressedControl = pointOver(e->pos());
    switch(pressedControl) {
	case SLIDER:
		 if (direction == Horizontal)
		     clickOffset = e->pos().x() - sliderPos;
		 else
		     clickOffset = e->pos().y() - sliderPos;
		 break;
	case NONE:
	         break;
	default:
                 drawControl((ScrollControl) pressedControl);
                 action((ScrollControl) pressedControl);
	         thresholdReached = FALSE;
                 timerID          = qStartTimer(thresholdTime,this);//!!!hanord
		 break;
    }
}

void QScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
    ScrollControl tmp = (ScrollControl) pressedControl;
    clickedAt      = FALSE;
    if (timerID != 0)
      qKillTimer(timerID);	// hanord!!! QObject::killTimer
    mouseMoveEvent(e);
    pressedControl = NONE;
    
    switch(tmp) {
	case SLIDER:
	         directSetValue(calculateValueFromSlider());
	         if (value() != previousValue())
	             emit newValue(value());
		 break;
	case ADD_LINE:
	case SUBTRACT_LINE:
	         drawControl((ScrollControl) tmp);
		 break;
	default:
	         break;
    }
}

void QScrollBar::mouseMoveEvent(QMouseEvent *e)
{
    int           newSliderPos;
    
    if (pressedControl == SLIDER) {
	if (direction == Horizontal)
   	    newSliderPos = e->pos().x() - clickOffset;
	else
   	    newSliderPos = e->pos().y() - clickOffset;
	if (newSliderPos < sliderMinPos())
	    newSliderPos = sliderMinPos();
	else
	    if (newSliderPos > sliderMaxPos())
	        newSliderPos = sliderMaxPos();
	if (newSliderPos != sliderPos) {
	    int newVal = sliderPosToRangeValue(newSliderPos);
	    if (track && newVal != value()) {
	        directSetValue(newVal);
		emit newValue(value());
	    }
	    moveSlider(newSliderPos);
	}
    }
}


QScrollBar::ScrollControl QScrollBar::pointOver(const QPoint &p)
{
    if (addButton().contains(p))
	return ADD_LINE;
    if (subtractButton().contains(p))
	return SUBTRACT_LINE;
    if (slider().contains(p))
	return SLIDER;
    if (clientRect().contains(p)) {
	int pos;
	if (direction == Horizontal)
	    pos = p.x();
	else
	    pos = p.y();
	if (pos < sliderPos && pos > sliderMinPos())
	    return SUBTRACT_PAGE;
	if (pos > sliderPos + sliderLength() &&
	        pos < sliderMaxPos() + sliderLength())
	    return ADD_PAGE;
    }
    return NONE;
}

int QScrollBar::border() const
{
    return 2;
}

int QScrollBar::length() const
{
    return direction == Horizontal ? clientRect().width()
                                   : clientRect().height();

}

int QScrollBar::controlWidth() const
{
    return direction == Horizontal ? clientRect().height() - border()*2 - 1
                                   : clientRect().width()  - border()*2 - 1;

}

int QScrollBar::buttonLength() const
{
    switch(guiStyle()) {
	case MotifStyle:
	         if (length() > controlWidth()*2 + 4 + 6 + 1)
		     return controlWidth();
		 else
		     return (length() - 5 - 6) / 2 - 1;
	default:
	         return 16;
    }
}

int QScrollBar::addButtonStart() const
{
    return length() - buttonLength() - border();
}

int QScrollBar::sliderMinPos() const
{
    return buttonLength() + border() + 2;
}

int QScrollBar::sliderMaxPos() const
{
    return addButtonStart() - sliderLength() - 2;
}

int QScrollBar::sliderLength() const
{
    switch(guiStyle()) {
	case MotifStyle: {
                 int tmp, maxLen;
		
                 maxLen = length() - 2*buttonLength() - border()*2;

                 if (maxValue() == minValue())
                     return maxLen;
                 tmp = (maxLen * pageStep()) /
                           (maxValue() - minValue() + pageStep());
		 if (tmp > maxLen)
		     tmp = maxLen;
		 return tmp > 5 ? tmp : 6;
	     }
	default:
	         return 16;
    }
}

QRect QScrollBar::addButton() const
{
    if (direction == Horizontal)
        return QRect(addButtonStart(),border(),
	                 buttonLength(),controlWidth());
    else
        return QRect(border(),addButtonStart(),
	                 controlWidth(),buttonLength());
}

QRect QScrollBar::subtractButton() const
{
    if (direction == Horizontal)
        return QRect(border(),border(),buttonLength(),controlWidth());
    else
        return QRect(border(),border(),controlWidth(),buttonLength());
}

QRect QScrollBar::slider() const
{
    if (direction == Horizontal)
        return QRect(sliderPos,border(),sliderLength(),controlWidth());
    else
        return QRect(border(),sliderPos,controlWidth(),sliderLength());
}



int QScrollBar::rangeValueToSliderPos(int val) const
{
    if (maxValue() == minValue())
        return sliderMinPos();
    return (int) (1.0*(val - minValue())/(maxValue() - minValue())*
		  (sliderMaxPos() - sliderMinPos()) + sliderMinPos() + 0.5);
}

int QScrollBar::sliderPosToRangeValue(int pos) const
{
    if (pos == sliderMinPos() || sliderMaxPos() == sliderMinPos())
        return minValue();
    if (pos == sliderMaxPos())
        return maxValue();
    return (int) (1.0*(pos - sliderMinPos())/(sliderMaxPos() - sliderMinPos())*
                      (maxValue() - minValue()) + minValue() + 0.5);
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = rangeValueToSliderPos(value());
}

int QScrollBar::calculateValueFromSlider() const
{
    return sliderPosToRangeValue(sliderPos);
}


void QScrollBar::action(ScrollControl control)
{
    switch(control) {
	case ADD_LINE:
	         addLine();
		 break;
	case SUBTRACT_LINE:
	         subtractLine();
		 break;
	case ADD_PAGE:
	         addPage();
		 break;
	case SUBTRACT_PAGE:
	         subtractPage();
		 break;
    }
}

void QScrollBar::drawControl(ScrollControl control) const
{
    QPainter p;

    p.begin(this);
    p.setBackgroundColor(foregroundColor());

    drawControl(control,p);
    p.end();
}

void QScrollBar::drawControl(ScrollControl control,QPainter &p) const
{
    switch (guiStyle()) {
	case MotifStyle:
	         drawMotifControl(control,p);
	         break;
	case WindowsStyle:
	         drawWindowsControl(control,p);
	         break;
	case MacStyle:
	         drawMacControl(control,p);
	         break;
    }
}


void QScrollBar::drawMotifControl(ScrollControl control,QPainter &p) const
{
    QBrush   b(foregroundColor());
    QPen     pen(foregroundColor().light(),2);
    QPoint   pt;
    QPointArray arrow(3);

    ScrollControl activeControl = (ScrollControl) pressedControl;

    QRect    addB = addButton();
    QRect    subB = subtractButton();

    p.setBackgroundColor(foregroundColor());
    p.setPen(pen);

    p.setBrush(b);

    switch (control) {
        case ADD_LINE :
		 pt = addB.center();
	         if (activeControl == ADD_LINE)
		     pen.setColor(foregroundColor().dark());
		 if (direction == Horizontal) {
	             arrow.setPoint(0,addB.left(),addB.bottom());
	             arrow.setPoint(1,addB.right(),pt.y());
	             arrow.setPoint(2,addB.left(),addB.top());
		 } else {
		     arrow.setPoint(0,pt.x(),addB.bottom());
	             arrow.setPoint(1,addB.right(),addB.top());
	             arrow.setPoint(2,addB.left(),addB.top());
		 }
		 if (activeControl == ADD_LINE)
		     b.setColor(foregroundColor().dark(1.12));
		 else
		     b.setColor(foregroundColor());

		 pen.setStyle(NoPen);
                 p.drawPolygon(arrow);

                 pen.setStyle(SolidLine);
		
		 if (activeControl == ADD_LINE)
		     pen.setColor(foregroundColor().light());
		 else
		     pen.setColor(foregroundColor().dark());
		 p.moveTo(arrow[0]);
		 p.lineTo(arrow[1]);
		 if (activeControl == ADD_LINE)
		     pen.setColor(foregroundColor().dark());
		 else
		     pen.setColor(foregroundColor().light());
		 p.lineTo(arrow[2]);
		 p.lineTo(arrow[0]);

	         break;
        case ADD_PAGE :
	         break;
        case SUBTRACT_LINE :
		 pt = subB.center();
	         if (activeControl == SUBTRACT_LINE)
		     b.setColor(foregroundColor().dark(1.12));
		 if (direction == Horizontal) {
	             arrow.setPoint(0,subB.right(),
		                      subB.top());
	             arrow.setPoint(1,subB.right(),
		                      subB.bottom());
	             arrow.setPoint(2,subB.left(),pt.y());
                 } else {
	             arrow.setPoint(0,pt.x(),subB.top());
	             arrow.setPoint(1,subB.right(),
		                      subB.bottom());
	             arrow.setPoint(2,subB.left(),
		                      subB.bottom());
                 }

                 p.drawPolygon(arrow);
		
	         if (activeControl == SUBTRACT_LINE)
		     pen.setColor(foregroundColor().light());
		 else
		     pen.setColor(foregroundColor().dark());
		 p.moveTo(arrow[0]);
		 p.lineTo(arrow[1]);
		 p.lineTo(arrow[2]);
	         if (activeControl == SUBTRACT_LINE)
		     pen.setColor(foregroundColor().dark());
		 else
		     pen.setColor(foregroundColor().light());
		 p.lineTo(arrow[0]);
		 
	         break;
        case SUBTRACT_PAGE :
	         break;
        case FIRST :
	         break;
        case LAST :
	         break;
        case SLIDER : {
	         p.drawShadePanel(slider(),foregroundColor().light(),
		                           foregroundColor().dark(),2,2,TRUE);
	         break;
	         }
        case NONE :
	         break;
	     }
}

void QScrollBar::drawWindowsControl(ScrollControl control,QPainter &p) const
{
    REF(control);
    REF(p);
}

void QScrollBar::drawMacControl(ScrollControl control,QPainter &p) const
{
    REF(control);
    REF(p);
}

void QScrollBar::drawNeXTControl(ScrollControl control,QPainter &p) const
{
    REF(control);
    REF(p);
}

void QScrollBar::moveMotifSlider(const QRect &oldSlider,const QRect &newSlider)
{
    int   moveLength;
    int   newLineShort,newLineLong;
    int   top,bottom;
    bool  movedLeft;
    QRect erase,fill;
    int   newPos,oldPos;


    if (oldSlider == newSlider)
        return;

    if (direction == Horizontal) {
        oldPos = oldSlider.left();
        newPos = newSlider.left();
    } else {
        oldPos = oldSlider.top();
        newPos = newSlider.top();
    }

    QPainter p;
    
    QBrush   b(foregroundColor().dark(1.12));
    QPen     pen;
    QRect    tmp;
    int      intersectWidth;


    p  .begin(this);
    p  .setBrush(b);
    p  .setPen(pen);
    pen.setStyle(NoPen);
    p.setBackgroundColor(foregroundColor());

    tmp = newSlider.intersect(oldSlider);
    if (direction == Horizontal)
        intersectWidth = tmp.width();
    else
        intersectWidth = tmp.height();
    

    if (intersectWidth <= 2) {
	p.drawRect(oldSlider);
	p.drawShadePanel(newSlider,foregroundColor().light(),
	                           foregroundColor().dark(),2,2,TRUE);
	p.end();
        sliderPos = newPos;
	return;
    }

    if (direction == Horizontal) {
	top    = newSlider.top();
	bottom = newSlider.bottom();
    
	if (newPos < oldPos) {
	    movedLeft    = TRUE;
	    moveLength   = oldPos - newPos;
	    newLineShort = newSlider.right();
	    newLineLong  = oldSlider.left();
	    erase        = QRect(QPoint(newSlider.right() + 1,newSlider.top()),
				 QPoint(oldSlider.right(),newSlider.bottom()));
	    fill         = QRect(QPoint(newSlider.left()  + 2,top    + 2),
				 QPoint(oldSlider.left()  + 2,bottom - 2));
	} else {
	    movedLeft    = FALSE;
	    moveLength   = newPos - oldPos;
	    newLineShort = newSlider.left();
	    newLineLong  = oldSlider.right();
	    erase        = QRect(QPoint(oldSlider.left() ,oldSlider.top()),
				QPoint(newSlider.left()-1,newSlider.bottom()));
	    fill         = QRect(QPoint(oldSlider.right() - 2,top    + 2),
				QPoint(newSlider.right() - 2,bottom - 2));
	}
    } else {
	top    = newSlider.left();
	bottom = newSlider.right();
    
	if (newPos < oldPos) {
	    movedLeft    = TRUE;
	    moveLength   = oldPos - newPos;
	    newLineShort = newSlider.bottom();
	    newLineLong  = oldSlider.top();
	    erase        = QRect(QPoint(newSlider.left(),newSlider.bottom()+1),
				 QPoint(newSlider.right(),oldSlider.bottom()));
	    fill         = QRect(QPoint(top    + 2,newSlider.top()  + 2),
				 QPoint(bottom - 2,oldSlider   .top()  + 2));
	} else {
	    movedLeft    = FALSE;
	    moveLength   = newPos - oldPos;
	    newLineShort = newSlider.top();
	    newLineLong  = oldSlider.bottom();
	    erase        = QRect(QPoint(oldSlider.left(),oldSlider.top()),
				 QPoint(newSlider.right(),newSlider.top() -1));
	    fill         = QRect(QPoint(top    + 2,oldSlider.bottom() - 2),
				 QPoint(bottom - 2,newSlider.bottom() - 2));
	}
    }
    
    pen.setStyle(NoPen);

    p.drawRect(erase);
    
    b.setColor(foregroundColor());
    p.drawRect(fill);

    pen.setStyle(SolidLine);

    if (direction == Horizontal) {
	if (movedLeft) {
	    pen.setColor(foregroundColor().light());
	    p.moveTo(newLineLong   ,top       );
	    p.lineTo(newPos        ,top       );
	    p.lineTo(newPos        ,bottom - 1);
	    p.moveTo(newLineLong   ,top    + 1);
	    p.lineTo(newPos + 1    ,top    + 1);
	    p.lineTo(newPos + 1    ,bottom - 2);
	    pen.setColor(foregroundColor().dark());
	    p.drawLine(newLineLong     ,bottom    ,newPos         ,bottom    );
	    p.drawLine(newLineLong     ,bottom - 1,newPos      + 1,bottom - 1);
	    p.drawLine(newLineShort    ,top      ,newLineShort    ,bottom - 2);
	    p.drawLine(newLineShort - 1,top   + 1,newLineShort - 1,bottom - 2);
	} else {
	    pen.setColor(foregroundColor().dark());
	    p.moveTo(newLineLong          ,bottom    );
	    p.lineTo(newSlider.right()    ,bottom    );
	    p.lineTo(newSlider.right()    ,top    - 1);
	    p.moveTo(newLineLong          ,bottom - 1);
	    p.lineTo(newSlider.right() - 1,bottom - 1);
	    p.lineTo(newSlider.right() - 1,top    - 2);
	    pen.setColor(foregroundColor().light());
	    p.drawLine(newLineLong  - 1,top    ,newSlider.right() - 1,top    );
	    p.drawLine(newLineLong  - 1,top + 1,newSlider.right() - 2,top + 1);
	    p.drawLine(newLineShort    ,top + 2,newLineShort    ,bottom - 1);
	    p.drawLine(newLineShort + 1,top + 2,newLineShort + 1,bottom - 2);
	}
    } else {
	if (movedLeft) {
	    pen.setColor(foregroundColor().light());
	    p.moveTo(top,newLineLong);
	    p.lineTo(top,newPos);
	    p.lineTo(bottom - 1,newPos);
	    p.moveTo(top + 1,newLineLong);
	    p.lineTo(top + 1,newPos + 1);
	    p.lineTo(bottom - 2,newPos + 1);
	    pen.setColor(foregroundColor().dark());
	    p.drawLine(bottom,newLineLong,bottom,newPos );
	    p.drawLine(bottom - 1,newLineLong ,bottom - 1,newPos + 1);
	    p.drawLine(top, newLineShort ,bottom - 2,newLineShort);
	    p.drawLine(top + 1,newLineShort - 1,bottom - 2,newLineShort - 1);
	} else {
	    pen.setColor(foregroundColor().dark());
	    p.moveTo(bottom,newLineLong);
	    p.lineTo(bottom,newSlider.bottom());
	    p.lineTo(top - 1,newSlider.bottom());
	    p.moveTo(bottom - 1,newLineLong);
	    p.lineTo(bottom - 1,newSlider.bottom() - 1);
	    p.lineTo(top - 2,newSlider.bottom() - 1);
	    pen.setColor(foregroundColor().light());
	    p.drawLine(top,newLineLong  - 1,top,newSlider.bottom() - 1);
	    p.drawLine(top + 1,newLineLong  - 1,top + 1,newSlider.bottom() -2);
	    p.drawLine(top + 2,newLineShort    ,bottom - 1,newLineShort);
	    p.drawLine(top + 2,newLineShort + 1,bottom - 2,newLineShort  + 1);
	}
    }
    p.end();

    sliderPos = newPos;
}

void QScrollBar::moveSlider(int newSliderPos)
{
    QRect newSlider;

    if (direction == Horizontal)
        newSlider.setRect(newSliderPos,border(),sliderLength(),controlWidth());
    else
        newSlider.setRect(border(),newSliderPos,controlWidth(),sliderLength());
    
    moveSlider(slider(),newSlider);
}

void QScrollBar::moveSlider(const QRect &oldSlider,const QRect &newSlider)
{
    moveMotifSlider(oldSlider,newSlider);
}
