/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnumber.h#1 $
**
** Definition of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QLCDNUM_H
#define QLCDNUM_H

#include "qwidget.h"

class QBitArray;


class QLCDNumber : public QWidget		// LCD number widget
{
    Q_OBJECT
public:
    QLCDNumber(QView *p,unsigned short noOfDigits = 1);
   ~QLCDNumber();

    enum Mode {HEXADECIMAL, DECIMAL, OCTAL, BINARY};

    bool     overflow(double num);
    bool     overflow(long   num);
    
    QLCDNumber::Mode mode(){return base;}

slots:
    void     display(int    num) {display((long)   num);}
    void     display(long   num);
    void     display(float  num) {display((double) num);}
    void     display(double num);
    void     display(char *s);
    void     setMode(Mode m);
    void     smallDecimalPoint(bool b);

private:
    void     resizeEvent(QResizeEvent *e);
    void     paintEvent(QPaintEvent *e);

    void     drawString(char *s,QPainter &p,QBitArray *newPoints = NULL,
                                            bool       update    = TRUE);
    void     drawDigit(QPoint pos, char ch,QPainter &p, QPen &pen);
    void     drawDigit(QPoint pos, char oldCh,char newCh,
                       QPainter &p, QPen &pen);
    void     drawSegment(QPoint pos, int segmentNo,QPainter &p, QPen &pen,
			 bool erase = FALSE);
    void     eraseSegment(QPoint pos,int segmentNo, QPainter &p, QPen &pen)
                                {drawSegment(pos,segmentNo,p,pen,TRUE);}
    int     *getSegments(char ch);
    int      digitLength();
    int      segmentXLength();
    int      segmentYLength();
    int      xSpace();
    int      xOffset();
    int      yOffset();

    unsigned short nDigits;
    char      *digits;
    QBitArray *points;
    bool       smallPoint;
    Mode       base;
};


#endif // QLCDNUM_H

