/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangect.h#4 $
**
** Definition of QRangeControl class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRANGECT_H
#define QRANGECT_H


class QRangeControl
{
public:
    QRangeControl();
    QRangeControl(long minValue, long maxValue,
		  long lineStep, long pageStep,long value);
    void    setValue(long value);
    void    addPage();
    void    subtractPage();
    void    addLine();
    void    subtractLine();
    void    setRange(long minValue, long maxValue);
    void    setSteps(long line,long page);
    long    minValue() const;
    long    maxValue() const;
    long    lineStep() const;
    long    pageStep() const;
    long    value() const;

protected:
    void    directSetValue(long val);
    long    previousValue() const;

private:
    void adjustValue();
    virtual void valueChange();
    virtual void stepChange();
    virtual void rangeChange();

    long minVal,maxVal;
    long line,page;
    long val;
    long previousVal;
};


inline long QRangeControl::minValue() const
{
    return minVal;
}

inline long QRangeControl::maxValue() const
{
    return maxVal;
}

inline long QRangeControl::lineStep() const
{
    return line;
}

inline long QRangeControl::pageStep() const
{
    return page;
}

inline long QRangeControl::value() const
{
    return val;
}

inline long QRangeControl::previousValue() const
{
    return previousVal;
}

#endif // QRANGECT_H
