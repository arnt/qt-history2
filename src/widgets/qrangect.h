/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qrangect.h#1 $
**
** Definition of QRangeControl class
**
** Author  : Eirik Eng
** Created : 940427
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QRANGECT_H
#define QRANGECT_H


class QRangeControl
{
public:
    QRangeControl();
    QRangeControl(int minValue, int maxValue,
		  int lineStep, int pageStep,int value);
    void    setValue(int value);
    void    addPage();
    void    subtractPage();
    void    addLine();
    void    subtractLine();
    void    setRange(int minValue, int maxValue);
    void    setSteps(int line,int page);
    void    setRangeAndSteps(int minValue, int maxValue,
			     int lineStep, int pageStep);
    int	    minValue() const;
    int	    maxValue() const;
    int	    lineStep() const;
    int	    pageStep() const;
    int	    value() const;

protected:
    void    directSetValue(int val);
    int	    previousValue() const;
    
private:
	    void adjustValue();
    virtual void valueChange(){}
    virtual void stepChange(){}
    virtual void rangeChange(){}

    int minVal,maxVal;
    int line,page;
    int val;
    int previousVal;
};


inline int QRangeControl::minValue() const
{
    return minVal;
}

inline int QRangeControl::maxValue() const
{
    return maxVal;
}

inline int QRangeControl::lineStep() const
{
    return line;
}

inline int QRangeControl::pageStep() const
{
    return page;
}

inline int QRangeControl::value() const
{
    return val;
}

inline int QRangeControl::previousValue() const
{
    return previousVal;
}


#endif // QRANGECT_H
