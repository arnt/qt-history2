/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnum.cpp#1 $
**
** Implementation of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qlcdnum.h"
#include "qbitarry.h"
#include "qpainter.h"
#include <stdio.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlcdnum.cpp#1 $";
#endif


QLCDNumber::QLCDNumber(QView *p,unsigned short noOfDigits)
    :QWidget(p)
{
    initMetaObject();
    base       = DECIMAL;
    smallPoint = FALSE;
    if (noOfDigits == 0)
        noOfDigits = 1;
    if (nDigits > 99) {
        warning("QLCDNumber: Maximum 99 digits allowed.");
	noOfDigits = 99;
    }
    digits  = new char[noOfDigits];
    points  = new QBitArray(noOfDigits);
    nDigits = noOfDigits;
    for (int i = 0 ; i < nDigits ; i++) {
	points->clearBit(i);
        digits[i] = ' ';
    }
    setBackgroundColor(lightGray);
}

QLCDNumber::~QLCDNumber()
{
    delete digits;
    delete points;
}

bool QLCDNumber::overflow(double num)
{
    int    i;
    double limit;

    if (base != DECIMAL)
        if (num > 1.0*65536*32768 - 1 || num < -1.0*65536*32768)
	    return TRUE;

    switch(base) {
	case HEXADECIMAL:
	         limit = 1;
	         for(i = 0 ; i < nDigits ; i++)
		     limit = limit*16;
	         if (num > limit || num < -limit/16)
		     return TRUE;
		 else
		     return FALSE;
	case DECIMAL:
		 return FALSE;            // Just do it!
	case OCTAL:
	         limit = 1;
	         for(i = 0 ; i < nDigits ; i++)
		     limit = limit*8;
	         if (num > limit || num < -limit/8)
		     return TRUE;
		 else
		     return FALSE;
	case BINARY:
	         limit = 1;
	         for(i = 0 ; i < nDigits ; i++)
		     limit = limit*2;
	         if (num > limit || num < -limit/2)
		     return TRUE;
		 else
		     return FALSE;
    }
    return FALSE;
}

bool QLCDNumber::overflow(long num)
{
    return overflow((double) num);
}

void QLCDNumber::display(long num)
{
    int  i;
    char buffer[100];

    if (overflow(num)) {
	display("Error");
	return;
    }

    bool negative;

    if (num < 0) {
	negative = TRUE;
	num = -num;
    } else {
	negative = FALSE;
    }

    switch(base) {
	case HEXADECIMAL:
                 sprintf(buffer,"%*lx",nDigits,num);
		 break;
	case DECIMAL:
                 sprintf(buffer,"%*li",nDigits,num);
		 break;
	case OCTAL:
                 sprintf(buffer,"%*lo",nDigits,num);
		 break;
	case BINARY: {
	         for (i = nDigits - 1 ; i >= 0 ; i--) {
		    if (!num && i != nDigits - 1) {
		        buffer[i] = ' ';
		    } else {
    		        if (num & 1)
		            buffer[i] = '1';
		        else
		            buffer[i] = '0';
		        num = num >> 1;
		    }
		 }
		 buffer[nDigits] = '\0';
		 break;
	     }
    }
    if (negative) {
	for (i = 0 ; i < strlen(buffer) ; i++) {
	    if (buffer[i] != ' ') {
		if (i != 0) {
		    buffer[i - 1] = '-';
		} else {
		    display("Error");
		    return;
		}
		break;
	    }
	}
    }        

//    fprintf(stderr,"sprintf gir: [%s]\n", buffer);

/*    if (strlen(buffer) <= nDigits) {
        QPainter p;
	
        p.begin(this);
        drawString(buffer,p);
        p.end();
    } else {
*/       display (buffer);
//    }
}

void QLCDNumber::display(double num)
{
    char buffer[100];

    if (overflow(num)) {
        display("Error");
	return;
    }
    
    if (base != DECIMAL) {
        display((long) num);
	return;
    }

    sprintf(buffer,"%*g",nDigits,num);
//    fprintf(stderr,"sprintf gir: [%s]\n", buffer);

    int len = strlen(buffer);

    for(int i = 0 ; i < len ; i++) {
	
        if (buffer[i] == 'e' && i != len - 1)
	    if (buffer[i + 1] == '+') {
		buffer[i    ] = ' ';
		buffer[i + 1] = 'e';
		break;
	    }
    }

    display (buffer);
}

void QLCDNumber::display(char *s)
{
    QPainter p;
    char *buffer = new char[nDigits + 1];
    buffer[nDigits] = '\0';

//    fprintf(stderr,"s = [%s] ",s);

    int i;
    int len = strlen(s);

    p.begin(this);
    if (!smallPoint) {
	if (len >= nDigits) {
	    for(int i = 0 ; i < nDigits ; i++)
		buffer[i] = s[len - nDigits + i];
	} else {
	    for(i = 0 ; i < nDigits - len ; i++)
		buffer[i] = ' ';
	    for(i = 0 ; i < len ; i++)
		buffer[nDigits - len + i] = s[i];
	}
        drawString(buffer,p); 
   } else {
        QBitArray *newPoints = new QBitArray(nDigits);
	int  index           = -1;
	bool lastWasPoint    = TRUE;

	newPoints->clearBit(0);
	for (i = 0 ; i < len ; i++) {
	    if (s[i] == '.') {
		if (lastWasPoint) {
		    if (index == nDigits - 1)
		        break;
		    index++;
		    buffer[index] = ' ';
		}
		newPoints->setBit(index);
		lastWasPoint = TRUE;
	    } else {
		if (index == nDigits - 1)
		    break;
		index++;
		buffer[index] = s[i];
		newPoints->clearBit(index);
		lastWasPoint = FALSE;
	    }
	}
	if (index < (int)nDigits - 1) {
	    for(i = index ; i >= 0 ; i--) {
		buffer[nDigits - 1 - index + i] = buffer[i];
		newPoints->setBit(nDigits - 1 - index + i,
		 	              newPoints->testBit(i));
	    }
	    for(i = 0 ; i < nDigits - index - 1; i++) {
		buffer[i] = ' ';
		newPoints->clearBit(i);
	    }
	}
        drawString(buffer,p,newPoints);
	delete newPoints;
    }
//    fprintf(stderr,"buffer = [%s] \n",buffer);
    
    p.end();
    
    delete buffer;
}

void QLCDNumber::setMode(Mode m)
{
    if (base == m)
        return;
    base = m;
    display("");
}

void QLCDNumber::smallDecimalPoint(bool b)
{
    if (smallPoint == b)
        return;
    smallPoint = b;
    display("");
}

void QLCDNumber::resizeEvent(QResizeEvent *e)
{
    e = e;

/*
    fprintf(stderr, "segmentYLength = %s\n"
                    "segmentXLength = %s\n"
                    "xSpace         = %s\n"
                    "segmentWidth   = %s\n"
                    "xOffset        = %s\n"
                    "yOffset        = %s\n"
                    "sz.getHeight() = %s\n"
                    "sz.getWidth()  = %s\n",
		    segmentYLength,
		    segmentXLength,
		    xSpace,
		    segmentWidth,
		    xOffset,
		    yOffset,
		    sz.getHeight(),
		    sz.getWidth());
*/

}

void QLCDNumber::paintEvent(QPaintEvent *)
{
    QPainter p;
    QColor tc, bc;

    p.begin(this);
    tc = backgroundColor().dark();
    bc = backgroundColor().light();
    p.drawShadePanel( QRect(QPoint(0,0),clientSize()),tc, bc, 1, 1 );
    if (smallPoint)
        drawString(digits,p,points,FALSE);
    else
        drawString(digits,p,NULL,FALSE);
    p.end();
}

void QLCDNumber::drawString(char *s,QPainter &p,
                            QBitArray *newPoints, bool update)
{
    QPoint  pos;
    QPen    pen;

    int     xOffset,yOffset;
    int     xLen = digitLength();
    int     len = strlen(s);

    xOffset = (clientSize().width() - (nDigits*(digitLength()) - xSpace()))/2;

    if (segmentYLength() < segmentXLength())
        yOffset = (clientSize().height() - 2*segmentYLength() - 5)/2;
    else
        yOffset = (clientSize().height() - 2*segmentXLength() - 5)/2;

    p.setPen(pen);

    for (int i = 0 ; i < nDigits ; i++) {
	pos = QPoint(xOffset + xLen*i,yOffset);
	if (update)
	    drawDigit(pos,digits[i],s[i],p,pen);
	else
	    drawDigit(pos,s[i],p,pen);
	if (newPoints) {
	    char newPoint = newPoints->testBit(i) ? '.' : ' ';
	    if (update) {
		char oldPoint = points   ->testBit(i) ? '.' : ' ';
		drawDigit(pos,oldPoint,newPoint,p,pen);
	    } else {
		drawDigit(pos,newPoint,p,pen);
	    }
	}
    }
    strncpy(digits,s,nDigits);
    if (newPoints)
	*points = *newPoints;
}

void QLCDNumber::drawDigit(QPoint pos, char ch, QPainter &p, QPen &pen)
{
    int *segs = getSegments(ch);
    
    for (int i = 0 ; segs[i] != -1 ; i++)
	drawSegment(pos,segs[i],p,pen);
}

void QLCDNumber::drawDigit(QPoint pos, char oldCh,char newCh,
                           QPainter &p, QPen &pen)
{
    int  updates[14][2];
    int  nErases;
    int  nUpdates;
    int *segs;
    int  i,j;

    const int erase      = 0;
    const int draw       = 1;
    const int leaveAlone = 2;

    segs     = getSegments(oldCh);
    for (nErases  = 0 ; segs[nErases] != -1 ; nErases++) {
	updates[nErases][0] = erase;              // Get segments to erase.
	updates[nErases][1] = segs[nErases];
    }
    nUpdates = nErases;

    segs     = getSegments(newCh);
    for(i = 0 ; segs[i] != -1 ; i++) {
	for (j = 0 ; j < nErases ; j++)
	    if (segs[i] == updates[j][1]) {  // Same segment ?
	        updates[j][0] = leaveAlone;  // Yes, already correct on screen.
		break;
	    }
	if (j == nErases) {
	    updates[nUpdates][0] = draw;
	    updates[nUpdates][1] = segs[i];
	    nUpdates++;
	}
    }
    for (i = 0 ; i < nUpdates ; i++) {
	if (updates[i][0] == draw)
	    drawSegment(pos,updates[i][1],p,pen);
	if (updates[i][0] == erase)
	    eraseSegment(pos,updates[i][1],p,pen);
    }
}

void QLCDNumber::drawSegment(QPoint pos,int segmentNo,QPainter &p, QPen &pen,
                             bool erase)
{
    QPoint pt = pos;
    QColor lightColor,darkColor;

    if (erase){
        lightColor = backgroundColor();
        darkColor  = lightColor;
    } else {
        lightColor = backgroundColor().light();
        darkColor  = backgroundColor().dark();
    }

    int yLen  = segmentYLength();
    int xLen  = segmentXLength();

    if (xLen > yLen)
        xLen = yLen;
    else
        yLen = xLen;

    int width = xLen*2/9;

    #define LINETO(X,Y) p.lineTo(QPoint(pt.x() + (X),pt.y() + (Y)))
    #define LIGHT pen.setColor(lightColor)
    #define DARK  pen.setColor(darkColor)

    switch (segmentNo) {
        case 0 : p.moveTo(pt);
		 LIGHT;
                 LINETO(xLen,0);
		 DARK;
                 LINETO(xLen - width,width);
                 LINETO(width,width);
                 LINETO(0,0);
                 break;
        case 1 : pt.y() = pt.y() + 1;
	         p.moveTo(pt);
		 LIGHT;
                 LINETO(width,width);
		 DARK;
                 LINETO(width,yLen - width/2);
                 LINETO(0,yLen);
		 LIGHT;
                 LINETO(0,0);
                 break;
        case 2 : pt.x() = pt.x() + xLen;
	         pt.y() = pt.y() + 1;
                 p.moveTo(pt);
		 DARK;
                 LINETO(0,yLen);
                 LINETO(-width,yLen - width/2);
		 LIGHT;
                 LINETO(-width,width);
                 LINETO(0,0);
                 break;
        case 3 : pt.y() = pt.y() + yLen + 2;
                 p.moveTo(pt);
		 LIGHT;
                 LINETO(width,-width/2);
                 LINETO(xLen - width,-width/2);
                 LINETO(xLen,0);
		 DARK;
                 LINETO(xLen - width,width/2);
                 LINETO(width,width/2);
                 LINETO(0,0);
                 break;
        case 4 : pt.y() = pt.y() + yLen + 3;
	         p.moveTo(pt);
		 LIGHT;
                 LINETO(width,width/2);
		 DARK;
                 LINETO(width,yLen - width);
                 LINETO(0,yLen);
		 LIGHT;
                 LINETO(0,0);
                 break;
        case 5 : pt.x() = pt.x() + xLen;
	         pt.y() = pt.y() + yLen + 3;
                 p.moveTo(pt);
		 DARK;
                 LINETO(0,yLen);
                 LINETO(-width,yLen - width);
		 LIGHT;
                 LINETO(-width,width/2);
                 LINETO(0,0);
                 break;
        case 6 : pt.y() = pt.y() + 2*yLen + 4;
                 p.moveTo(pt);
		 LIGHT;
                 LINETO(width,-width);
                 LINETO(xLen - width,-width);
                 LINETO(xLen,0);
		 DARK;
                 LINETO(0,0);
                 break;
        case 7 : if (smallPoint)
	             pt.x() = pt.x() + xLen + xSpace()/2 - width/2 + 1;
		 else
	             pt.x() = pt.x() + xLen/2 + 1;
                 pt.y() = pt.y() + 2*yLen + 4;
                 p.moveTo(pt);
		 DARK;
                 LINETO(width,0);
                 LINETO(width,-width);
		 LIGHT;
                 LINETO(0,-width);
                 LINETO(0,0);
                 break;
        case 8 : pt.y() = pt.y() + yLen/2 + width;
                 pt.x() = pt.x() + xLen/2 - width/2 + 1;
                 p.moveTo(pt);
		 DARK;
                 LINETO(width,0);
                 LINETO(width,-width);
		 LIGHT;
                 LINETO(0,-width);
                 LINETO(0,0);
                 break;
        case 9 : pt.y() = pt.y() + 3*yLen/2 + width;
                 pt.x() = pt.x() + xLen/2 - width/2 + 1;
                 p.moveTo(pt);
		 DARK;
                 LINETO(width,0);
                 LINETO(width,-width);
		 LIGHT;
                 LINETO(0,-width);
                 LINETO(0,0);
                 break;
        default :
	         fprintf(stderr,
		         "QLCDNumber::drawSegment: Internal error\n"
		         "    Illegal segment ID: %s\n",
			 segmentNo);
    }

    #undef LINETO
    #undef LIGHT
    #undef DARK
}

int *QLCDNumber::getSegments(char ch)
{
    static int segments[30][8] = {{ 0, 1, 2, 4, 5, 6,-1, 0},  // 0    0 / O
                                  { 2, 5,-1, 0, 0, 0, 0, 0},  // 1    1
                                  { 0, 2, 3, 4, 6,-1, 0, 0},  // 2    2
                                  { 0, 2, 3, 5, 6,-1, 0, 0},  // 3    3
                                  { 1, 2, 3, 5,-1, 0, 0, 0},  // 4    4
                                  { 0, 1, 3, 5, 6,-1, 0, 0},  // 5    5
                                  { 0, 1, 3, 4, 5, 6,-1, 0},  // 6    6
                                  { 0, 2, 5,-1, 0, 0, 0, 0},  // 7    7
                                  { 0, 1, 2, 3, 4, 5, 6,-1},  // 8    8
                                  { 0, 1, 2, 3, 5, 6,-1, 0},  // 9    9 / g
                                  { 3,-1, 0, 0, 0, 0, 0, 0},  // 10   -
                                  { 7,-1, 0, 0, 0, 0, 0, 0},  // 11   .
                                  { 0, 1, 2, 3, 4, 5,-1, 0},  // 12   A
                                  { 1, 3, 4, 5, 6,-1, 0, 0},  // 13   B
                                  { 0, 1, 4, 6,-1, 0, 0, 0},  // 14   C
                                  { 2, 3, 4, 5, 6,-1, 0, 0},  // 15   D 
                                  { 0, 1, 3, 4, 6,-1, 0, 0},  // 16   E 
                                  { 0, 1, 3, 4,-1, 0, 0, 0},  // 17   F
                                  { 1, 3, 4, 5,-1, 0, 0, 0},  // 18   h
                                  { 1, 2, 3, 4, 5,-1, 0, 0},  // 19   H
                                  { 1, 4, 6,-1, 0, 0, 0, 0},  // 20   L
                                  { 3, 4, 5, 6,-1, 0, 0, 0},  // 21   o
                                  { 0, 1, 2, 3, 4,-1, 0, 0},  // 22   P
                                  { 3, 4,-1, 0, 0, 0, 0, 0},  // 23   r
                                  { 0, 1, 3, 5, 6,-1, 0, 0},  // 24   S
                                  { 4, 5, 6,-1, 0, 0, 0, 0},  // 25   u
                                  { 1, 2, 4, 5, 6,-1, 0, 0},  // 26   U
                                  { 1, 2, 3, 5, 6,-1, 0, 0},  // 27   Y
                                  { 8, 9,-1, 0, 0, 0, 0, 0}, // 28   :
                                  {-1, 0, 0, 0, 0, 0, 0, 0}}; // 29   empty

    if (ch >= '0' && ch <= '9')
        return segments[ch - '0'];
    if (ch >= 'A' && ch <= 'F')
        return segments[ch - 'A' + 12];
    if (ch >= 'a' && ch <= 'f')
        return segments[ch - 'a' + 12];
    if (ch == '-')
        return segments[10];
    if (ch == 'O')
        return segments[0];
    if (ch == 'g')
        return segments[9];
    if (ch == '.')
        return segments[11];
    if (ch == 'h')
        return segments[18];
    if (ch == 'H')
        return segments[19];
    if (ch == 'l' || ch == 'L')
        return segments[20];
    if (ch == 'o')
        return segments[21];
    if (ch == 'p' || ch == 'P')
        return segments[22];
    if (ch == 'r' || ch == 'R')
        return segments[23];
    if (ch == 's' || ch == 'S')
        return segments[24];
    if (ch == 'u')
        return segments[25];
    if (ch == 'U')
        return segments[26];
    if (ch == 'y' || ch == 'Y')
        return segments[27];
    if (ch == ':')
        return segments[28];
    return segments[29];
}

int QLCDNumber::digitLength()
{
    return (clientSize().width() - 11)/nDigits;
}

int QLCDNumber::segmentXLength()
{
    return digitLength() - xSpace();
}

int QLCDNumber::segmentYLength()
{
    return (clientSize().height() - 16)/2;
}

int QLCDNumber::xSpace()
{
    if (smallPoint)
        return digitLength()*4/9;
    else
        return digitLength()/5;
}

int QLCDNumber::xOffset()
{
    return  (clientSize().width()  -
                (nDigits*(digitLength()) - xSpace()))/2;
}

int QLCDNumber::yOffset()
{
    return  (clientSize().height() - 2*segmentYLength() - 5)/2;
}

