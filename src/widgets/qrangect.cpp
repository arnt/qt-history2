/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangect.cpp#4 $
**
** Implementation of QRangeControl class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qrangect.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qrangect.cpp#4 $";
#endif

/*! \class QRangeControl qrangect.h

  \brief The QRangeControl is an abstract class which provides a
  variable which can move within limits.

  This class is not yet documented.  Our <a
  href=http://www.troll.no/>home page</a> contains a pointer to the
  current version of Qt. */

#undef ABS
#define ABS(X) ((X) < 0) ? (-(X)) : (X)

QRangeControl::QRangeControl()
{
    minVal	= 0;
    maxVal	= 99;
    line	= 1;
    page	= 10;
    val		= 0;
    previousVal = 0;
}

QRangeControl::QRangeControl(long minValue, long maxValue,
			     long LineStep, long PageStep,
			     long value)
{
    minVal	= minValue;
    maxVal	= maxValue;
    line	= ABS(LineStep);
    page	= ABS(PageStep);
    val	        = value;
    previousVal = value;
    adjustValue();
}

void QRangeControl::setValue(long value)
{
    directSetValue(value);
    if (previousVal != val)
	valueChange();
}

void QRangeControl::directSetValue(long value)
{
    previousVal = val;
    val         = value;
    adjustValue();
}

void QRangeControl::addPage()
{
    previousVal = val;
    val         = val + page;
    if (val > maxVal)
	val = maxVal;
    if (previousVal != val)
	valueChange();
}

void QRangeControl::subtractPage()
{
    previousVal = val;
    val         = val - page;
    if (val < minVal)
	val = minVal;
    if (previousVal != val)
	valueChange();
}

void QRangeControl::addLine()
{
    previousVal = val;
    val         = val + line;
    if (val > maxVal)
	val = maxVal;
    if (previousVal != val)
	valueChange();
}

void QRangeControl::subtractLine()
{
    previousVal = val;
    val         = val - line;
    if (val < minVal)
	val = minVal;
    if (previousVal != val)
	valueChange();
}

void QRangeControl::adjustValue()
{
    if (val < minVal)
	val = minVal;
    if (val > maxVal)
	val = maxVal;
}

void QRangeControl::valueChange()
{

}

void QRangeControl::stepChange()
{

}

void QRangeControl::rangeChange()
{

}

void QRangeControl::setRange(long minValue, long maxValue)
{
    if (minValue == minVal && maxValue == maxVal)
        return;
    if (minValue > maxValue) {
	minVal = minValue;
	maxVal = minValue;
    } else {
	minVal = minValue;
	maxVal = maxValue;
    }
    long tmp = val;
    adjustValue();
    rangeChange();
    if (tmp != val) {
	previousVal = tmp;
	valueChange();
    }
}

void QRangeControl::setSteps(long lineStep,long pageStep)
{
    if (lineStep != line || pageStep != page) {
        line = ABS(lineStep);
        page = ABS(pageStep);
        stepChange();
    }
}
