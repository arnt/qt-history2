/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangecontrol.cpp#29 $
**
** Implementation of QRangeControl class
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

#include "qrangecontrol.h"
#include "qglobal.h"
#include <limits.h>

/*!
  \class QRangeControl qrangecontrol.h
  \brief The QRangeControl class provides an integer value within a range.

  \ingroup misc

  It was originally designed for the QScrollBar widget, but it can
  also be practical for other purposes.  QSlider, QSpinBox and QDial
  also inherit QRangeControl.  Here are the five main concepts the
  class has: <ul>

  <li> The current value.  This is the bounded integer that
  QRangeControl maintains.  value() returns this, and several
  functions including setValue() set it.

  <li> The minimum.  This is the lowest value value() can ever return.
  Returned by minValue(), set by setRange() or one of the
  constructors.

  <li> The maximum.  This is the highest value value() can ever return.
  Returned by maxValue(), set by setRange() or one of the
  constructors.

  <li> The line step.  This is the smaller of two natural steps
  QRangeControl provides, and typically corresponds to the user
  pressing an arrow key.  Returned by lineStep(), set using
  setSteps(), and the addLine() and subtractLine() allow easy
  movement of the current value by lineStep().

  <li> The page step.  This is the larger of two natural steps
  QRangeControl provides, and typically corresponds to the user
  pressing one of the PageUp and PageDown keys.  Returned by
  pageStep(), set using setSteps(), and the addPage() and
  substractPage() allow easy movement of the current value by
  pageStep().

  </ul>

  Note that unity (1) may be viewed as a third step size.  setValue()
  lets you set the current value to any integer in the allowed range,
  not just minValue()+n*lineStep() for integer values of n.  Some
  widgets may allow the user to set any value at all, others may just
  provide multiples of lineStep()/pageStep().  The choice is up to you.

  QRangeControl provides three virtual functions that are well-suited
  e.g. to updating the on-screen representation of range controls and
  emitting signals, namely valueChange(), rangeChange() and
  stepChange().

  Finally, QRangeControl provides a function called bound() to let you
  force arbitrary integers to within the range of a range control.

  We recommend that all widgets provide at least a signal called
  valueChanged(), and many widgets will want to provide addStep(),
  addPage(), substractStep() and substractPage() as slots.
*/


/*!
  Constructs a range control with min value 0, max value 99,
  line step 1, page step 10 and initial value 0.
*/

QRangeControl::QRangeControl()
{
    minVal  = 0;
    maxVal  = 99;
    line    = 1;
    page    = 10;
    val	    = prevVal = 0;
    d       = 0;
}

/*!
  Constructs a range control whose value can never be smaller than \a
  minValue or greater than \a maxValue, whose line step size is \a
  lineStep and page step size is \a pageStep, and whose value is
  initially \a value.

  \a value is forced to be within the legal range.
*/

QRangeControl::QRangeControl( int minValue, int maxValue,
			      int lineStep, int pageStep,
			      int value )
{
    minVal = minValue;
    maxVal = maxValue;
    line   = QABS( lineStep );
    page   = QABS( pageStep );
    val	   = prevVal = bound( value );
    d       = 0;
}


/*!
  \fn int QRangeControl::value() const

  Returns the current range control value.  This is guaranteed to be
  within the range [ minValue() ... maxValue() ].

  \sa setValue(), prevValue()
*/

/*!
  \fn int QRangeControl::prevValue() const

  Returns the previous value of the range control.  When the range
  control is initially created, this is the same as value().

  Note that prevValue() can be outside the legal range if a call to
  setRange() causes the current value to change.  (For example if the
  range was 0-1000 and the current value 500, setRange( 0, 400 ) makes
  value() return 400 and prevValue() 500.)

  \sa value() setRange()
*/

/*!
  Sets the range control value to \e value and forces it to be within
  the legal range.

  Calls the virtual valueChange() function if the new value is
  different from the previous value.
*/

void QRangeControl::setValue( int value )
{
    directSetValue( value );
    if ( prevVal != val )
	valueChange();
}

/*!
  Sets the range control value directly without calling valueChange().

  Forces the new value to be within the legal range.

  \sa setValue()
*/

void QRangeControl::directSetValue(int value)
{
    prevVal = val;
    val	    = bound( value );
}

/*!
  Equivalent to \code setValue( value()+pageStep() )\endcode plus a
  test for numerical overflow.

  \sa subtractPage()
*/

void QRangeControl::addPage()
{
    if ( value() + pageStep() > value() )
	setValue( value() + pageStep() );
    else
	setValue( maxValue() );
}

/*!
  Equivalent to \code setValue( value()-pageStep() )\endcode  plus a
  test for numerical underflow
  \sa addPage()
*/

void QRangeControl::subtractPage()
{
    if ( value() - pageStep() < value() )
	setValue( value() - pageStep() );
    else
	setValue( minValue() );
}

/*!
  Equivalent to \code setValue( value()+lineStep() )\endcode  plus a
  test for numerical overflow
  \sa subtractLine()
*/

void QRangeControl::addLine()
{
    if ( value() + lineStep() > value() )
	setValue( value() + lineStep() );
    else
	setValue( maxValue() );
}

/*!
  Equivalent to \code setValue( value()-lineStep() )\endcode plus a
  test for numerical underflow
  \sa addLine()
*/

void QRangeControl::subtractLine()
{
    if ( value() - lineStep() < value() )
	setValue( value() - lineStep() );
    else
	setValue( minValue() );
}


/*!
  \fn int QRangeControl::minValue() const

  Returns the current minimum value in the range.

  \sa setRange(), maxValue()
*/

/*!
  \fn int QRangeControl::maxValue() const

  Returns the current maximum value in the range.

  \sa setRange(), minValue()
*/

/*!
  Sets the range min value to \e minValue and the max value to \e
  maxValue.

  Calls the virtual rangeChange() function if one or both of the new
  min and max values are different from the previous setting.  Calls
  the virtual valueChange() function if the current value is adjusted
  because or was is outside the new range.

  If \a maxValue is smaller than \a minValue, \a minValue becomes the
  only legal value.

  \sa minValue(), maxValue()
*/

void QRangeControl::setRange( int minValue, int maxValue )
{
    if ( minValue == minVal && maxValue == maxVal )
	return;
    if ( minValue > maxValue ) {
#if defined(CHECK_RANGE)
	warning( "QRangeControl::setRange: minValue %d > maxValue %d",
		 minValue, maxValue );
#endif
	minVal = maxVal = minValue;
    } else {
	minVal = minValue;
	maxVal = maxValue;
    }
    int tmp = bound( val );
    rangeChange();
    if ( tmp != val ) {
	prevVal = tmp;
	valueChange();
    }
}


/*!
  \fn int QRangeControl::lineStep() const
  Returns the current line step.
  \sa setSteps(), pageStep()
*/

/*!
  \fn int QRangeControl::pageStep() const
  Returns the current page step.
  \sa setSteps(), lineStep()
*/

/*!
  Sets the range line step to \e lineStep and page step to \e pageStep.

  Calls the virtual stepChange() function if the new line step and/or
  page step are different from the previous setting.

  \sa lineStep() pageStep() setRange()
*/

void QRangeControl::setSteps(int lineStep,int pageStep)
{
    if (lineStep != line || pageStep != page) {
	line = QABS(lineStep);
	page = QABS(pageStep);
	stepChange();
    }
}


/*!
  This virtual function is called whenever the range control value
  changes.  You can reimplement it if you want to be notified when the
  value changes.  The default implementation does nothing.

  \sa setValue(), addPage(), subtractPage(), addLine(), subtractLine()
  rangeChange(), stepChange()
*/

void QRangeControl::valueChange()
{
}


/*!
  This virtual function is called whenever the range control range
  changes.  You can reimplement it if you want to be notified when the range
  changes.  The default implementation does nothing.

  \sa setRange(), valueChange(), stepChange()
*/

void QRangeControl::rangeChange()
{
}


/*!
  This virtual function is called whenever the range control step
  value changes.  You can reimplement it if you want to be notified
  when the step changes.  The default implementation does nothing.

  \sa setSteps(), rangeChange(), valueChange()
*/

void QRangeControl::stepChange()
{
}


/*!  Forces \a v within the range from minValue() to maxValue()
  inclusive, and returns the result.

  This function is provided so that you can easily force other numbers
  than value() into the allowed range.  You do not need to call it in
  order to use QRangeControl itself.

  \sa setValue() value() minValue() maxValue()
*/

int QRangeControl::bound( int v ) const
{
    if ( v < minVal )
	return minVal;
    if ( v > maxVal )
	return maxVal;
    return v;
}


/*!
  Converts \a val to a pixel position. minValue() maps to 0, maxValue()
  maps to \a space, and other values are distributed evenly in between.
  
  This function can handle the entire integer range without overflow.
*/

int QRangeControl::positionFromValue( int val, int space ) const
{
    if ( maxValue() > minValue() ) {
	uint range = maxValue() - minValue();
	uint d = val - minValue();
	int scale = 1;
	if ( range > uint(INT_MAX/4096) )
	     scale = 4096*2;
	return ( (d/scale) * space ) / (range/scale);
    } else {
	return 0;
    }
}


/*!
  Converts the pixel position \a pos to a value. 0 maps to minValue(),
  \a space maps to maxValue(), and other values are distributed evenly
  in between.

  This function can handle the entire integer range without overflow.
*/

int QRangeControl::valueFromPosition( int pos, int space ) const
{
    if ( space <= 0 || pos <= 0 )
	return minValue();
    if ( pos >= space )
	return maxValue();

    uint range = maxValue() - minValue();
    return  minValue() +  pos*(range/space) 
	+ (2 * pos * (range%space) + space) / (2*space) ;
    //equiv. to minValue() + ( p * r) / space + 0.5;

}
