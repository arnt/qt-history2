/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangecontrol.h#7 $
**
** Definition of QRangeControl class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRANGECT_H
#define QRANGECT_H


class QRangeControl
{
public:
    QRangeControl();
    QRangeControl( int minValue, int maxValue,
		   int lineStep, int pageStep, int value );

    int		value()		const;
    void	setValue( int );
    void	addPage();
    void	subtractPage();
    void	addLine();
    void	subtractLine();

    int		minValue()	const;
    int		maxValue()	const;
    void	setRange( int minValue, int maxValue );

    int		lineStep()	const;
    int		pageStep()	const;
    void	setSteps( int line, int page );

protected:
    void	directSetValue( int val );
    int		prevValue()	const;

private:
    void	adjustValue();
    virtual void valueChange();
    virtual void rangeChange();
    virtual void stepChange();

    int		minVal, maxVal;
    int		line, page;
    int		val, prevVal;
};


inline int QRangeControl::value() const
{ return val; }

inline int QRangeControl::prevValue() const
{ return prevVal; }

inline int QRangeControl::minValue() const
{ return minVal; }

inline int QRangeControl::maxValue() const
{ return maxVal; }

inline int QRangeControl::lineStep() const
{ return line; }

inline int QRangeControl::pageStep() const
{ return page; }


#endif // QRANGECT_H
