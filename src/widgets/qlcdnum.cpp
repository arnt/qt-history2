/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnum.cpp#3 $
**
** Implementation of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qlcdnum.h"
#include "qbitarry.h"
#include "qpainter.h"
#include <stdio.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlcdnum.cpp#3 $";
#endif


static QString long2string( long num, int base, int ndigits, bool *oflow )
{
    QString s;
    bool negative = FALSE;
    switch( base ) {
	case QLCDNumber::HEXADECIMAL:
	    s.sprintf( "%*lx", ndigits, num );
	    break;
	case QLCDNumber::DECIMAL:
	    if ( num < 0 ) {
		negative = TRUE;
		num = -num;
	    }
	    s.sprintf( "%*li", ndigits, num );
	    break;
	case QLCDNumber::OCTAL:
	    s.sprintf( "%*lo", ndigits, num );
	    break;
	case QLCDNumber::BINARY: {
	    char buf[42];
	    char *p = &buf[41];
	    ulong n = num;
	    int len = 0;
	    *p = '\0';
	    do {
		*--p = ((n&1)+'0');
		n >>= 1;
		len++;
	    } while ( n != 0 );
	    len = ndigits - len;
	    if ( len > 0 )
	    s.fill( ' ', len );
	    s += p;
	    }
	    break;
    }
    if ( negative ) {
	for ( int i=0; i<s.length(); i++ ) {
	    if ( s[i] != ' ' ) {
		if ( i != 0 ) {
		    s[i-1] = '-';
		}
		break;
	    }
	}
    }
    if ( oflow )
	*oflow = s.length() > ndigits;
    return s;
}


QString double2string( double num, int base, int ndigits, bool *oflow )
{
    QString s;
    if ( base != QLCDNumber::DECIMAL ) {
	bool of = num >= 2147483648.0 || num < -2147483648.0;
	if ( of ) {				// oops, 'long' overflow
	    if ( oflow )
		*oflow = TRUE;
	    return s;
	}
	s = long2string( (long)num, base, ndigits, 0 );
    }
    else {					// decimal base
	s.sprintf( "%*g", ndigits, num );
	int i = s.find('e');
	if ( i > 0 && s[i+1]=='+' ) {
	    s[i] = ' ';
	    s[i+1] = 'e';
	}
    }
    if ( oflow )
	*oflow = s.length() > ndigits;
    return s;
}


char *getSegments( char ch )			// gets list of segments for ch
{
    static char segments[30][8] =
       { { 0, 1, 2, 4, 5, 6,-1, 0},		// 0    0 / O
	 { 2, 5,-1, 0, 0, 0, 0, 0},		// 1    1
	 { 0, 2, 3, 4, 6,-1, 0, 0},		// 2    2
	 { 0, 2, 3, 5, 6,-1, 0, 0},		// 3    3
	 { 1, 2, 3, 5,-1, 0, 0, 0},		// 4    4
	 { 0, 1, 3, 5, 6,-1, 0, 0},		// 5    5
	 { 0, 1, 3, 4, 5, 6,-1, 0},		// 6    6
	 { 0, 2, 5,-1, 0, 0, 0, 0},		// 7    7
	 { 0, 1, 2, 3, 4, 5, 6,-1},		// 8    8
	 { 0, 1, 2, 3, 5, 6,-1, 0},		// 9    9 / g
	 { 3,-1, 0, 0, 0, 0, 0, 0},		// 10   -
	 { 7,-1, 0, 0, 0, 0, 0, 0},		// 11   .
	 { 0, 1, 2, 3, 4, 5,-1, 0},		// 12   A
	 { 1, 3, 4, 5, 6,-1, 0, 0},		// 13   B
	 { 0, 1, 4, 6,-1, 0, 0, 0},		// 14   C
	 { 2, 3, 4, 5, 6,-1, 0, 0},		// 15   D 
	 { 0, 1, 3, 4, 6,-1, 0, 0},		// 16   E 
	 { 0, 1, 3, 4,-1, 0, 0, 0},		// 17   F
	 { 1, 3, 4, 5,-1, 0, 0, 0},		// 18   h
	 { 1, 2, 3, 4, 5,-1, 0, 0},		// 19   H
	 { 1, 4, 6,-1, 0, 0, 0, 0},		// 20   L
	 { 3, 4, 5, 6,-1, 0, 0, 0},		// 21   o
	 { 0, 1, 2, 3, 4,-1, 0, 0},		// 22   P
	 { 3, 4,-1, 0, 0, 0, 0, 0},		// 23   r
	 { 0, 1, 3, 5, 6,-1, 0, 0},		// 24   S
	 { 4, 5, 6,-1, 0, 0, 0, 0},		// 25   u
	 { 1, 2, 4, 5, 6,-1, 0, 0},		// 26   U
	 { 1, 2, 3, 5, 6,-1, 0, 0},		// 27   Y
	 { 8, 9,-1, 0, 0, 0, 0, 0},		// 28   :
	 {-1, 0, 0, 0, 0, 0, 0, 0} };		// 29   empty

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


QLCDNumber::QLCDNumber( QView *p, int noOfDigits ) : QWidget( p )
{
    initMetaObject();
    base = DECIMAL;
    smallPoint = FALSE;
    if ( noOfDigits <= 0 )
        noOfDigits = 1;
    else if ( nDigits > 99 ) {
#if defined(CHECK_RANGE)
        warning( "QLCDNumber: Maximum 99 digits allowed" );
#endif
	noOfDigits = 99;
    }
    nDigits = noOfDigits;
    digits.fill( ' ', nDigits );
    points.fill( 0, nDigits );
    setBackgroundColor( lightGray );
}

QLCDNumber::~QLCDNumber()
{
}


bool QLCDNumber::overflow( long num ) const
{
    bool of;
    long2string( num, base, nDigits, &of );
    return of;
}

bool QLCDNumber::overflow( double num ) const
{
    bool of;
    double2string( num, base, nDigits, &of );
    return of;
}


void QLCDNumber::display( long num )
{
    bool of;
    QString s = long2string( num, base, nDigits, &of );
    if ( of )
	s = "Error";
    display( s );
}


void QLCDNumber::display( double num )
{
    bool of;
    QString s = double2string( num, base, nDigits, &of );
    if ( of )
	s = "Error";
    display( s );
}


void QLCDNumber::display( const char *s )
{
    QPainter p;
    QString buffer(nDigits+1);
    int i;
    int len = strlen(s);

    p.begin( this );
    if ( !smallPoint ) {
	if ( len >= nDigits ) {
	    for( i=0; i<nDigits; i++ )
		buffer[i] = s[len - nDigits + i];
	}
	else {
	    for( i=0; i<nDigits-len; i++ )
		buffer[i] = ' ';
	    for( i=0; i<len; i++ )
		buffer[nDigits - len + i] = s[i];
	}
        drawString( buffer, p );
   }
   else {
	int  index = -1;
	bool lastWasPoint = TRUE;
        QBitArray *newPoints = new QBitArray(nDigits);
	CHECK_PTR( newPoints );
	newPoints->clearBit(0);
	for ( i=0; i<len; i++ ) {
	    if ( s[i] == '.' ) {
		if ( lastWasPoint ) {
		    if ( index == nDigits - 1 )
		        break;
		    index++;
		    buffer[index] = ' ';
		}
		newPoints->setBit(index);
		lastWasPoint = TRUE;
	    }
	    else {
		if ( index == nDigits - 1 )
		    break;
		index++;
		buffer[index] = s[i];
		newPoints->clearBit(index);
		lastWasPoint = FALSE;
	    }
	}
	if ( index < nDigits - 1 ) {
	    for( i=index; i>=0; i-- ) {
		buffer[nDigits - 1 - index + i] = buffer[i];
		newPoints->setBit( nDigits - 1 - index + i,
				   newPoints->testBit(i) );
	    }
	    for( i=0; i<nDigits-index-1; i++ ) {
		buffer[i] = ' ';
		newPoints->clearBit(i);
	    }
	}
        drawString(buffer,p,newPoints);
	delete newPoints;
    }
    p.end();
}


void QLCDNumber::setMode(Mode m)
{
    if ( base == m )
        return;
    base = m;
    display( "" );
}

void QLCDNumber::smallDecimalPoint(bool b)
{
    if ( smallPoint == b )
        return;
    smallPoint = b;
    display( "" );
}


void QLCDNumber::resizeEvent( QResizeEvent * )
{
}

void QLCDNumber::paintEvent(QPaintEvent *)
{
    QPainter p;
    QColor tc, bc;

    p.begin(this);
    tc = backgroundColor().dark();
    bc = backgroundColor().light();
    p.drawShadePanel( clientRect(), tc, bc, 1, 1 );
    if ( smallPoint )
        drawString( digits, p, &points, FALSE );
    else
        drawString( digits, p, 0, FALSE );
    p.end();
}


void QLCDNumber::drawString( const char *s, QPainter &p,
			     QBitArray *newPoints, bool update )
{
    QPoint  pos;
    QPen    pen;
    int xOffset, yOffset;
    int xLen = digitLength();
    xOffset = (clientSize().width() - (nDigits*xLen - xSpace()))/2;
    if ( segmentYLength() < segmentXLength() )
        yOffset = (clientSize().height() - 2*segmentYLength() - 5)/2;
    else
        yOffset = (clientSize().height() - 2*segmentXLength() - 5)/2;
    p.setPen(pen);
    for ( int i=0;  i<nDigits; i++ ) {
	pos = QPoint(xOffset + xLen*i,yOffset);
	if ( update )
	    drawDigit(pos,digits[i],s[i],p,pen);
	else
	    drawDigit(pos,s[i],p,pen);
	if ( newPoints ) {
	    char newPoint = newPoints->testBit(i) ? '.' : ' ';
	    if ( update ) {
		char oldPoint = points.testBit(i) ? '.' : ' ';
		drawDigit(pos,oldPoint,newPoint,p,pen);
	    }
	    else
		drawDigit(pos,newPoint,p,pen);
	}
    }
    digits = s;
    if ( digits.length() > nDigits )
	digits.resize( nDigits );
    if ( newPoints )
	points = *newPoints;
}


void QLCDNumber::drawDigit( const QPoint &pos, char ch, QPainter &p, QPen &pen)
{
    char *segs = getSegments(ch);
    while ( *segs != -1 )
	drawSegment( pos, *segs++, p, pen );
}


void QLCDNumber::drawDigit( const QPoint &pos, char oldCh, char newCh,
			    QPainter &p, QPen &pen )
{
    int  updates[14][2];
    int  nErases;
    int  nUpdates;
    char *segs;
    int  i,j;

    const int erase      = 0;
    const int draw       = 1;
    const int leaveAlone = 2;

    segs = getSegments(oldCh);
    for ( nErases=0; segs[nErases] != -1; nErases++ ) {
	updates[nErases][0] = erase;		// get segments to erase
	updates[nErases][1] = segs[nErases];
    }
    nUpdates = nErases;
    segs = getSegments(newCh);
    for(i = 0 ; segs[i] != -1 ; i++) {
	for ( j=0;  j<nErases; j++ )
	    if ( segs[i] == updates[j][1] ) {	// same segment ?
	        updates[j][0] = leaveAlone;	// yes, already on screen
		break;
	    }
	if ( j == nErases ) {
	    updates[nUpdates][0] = draw;
	    updates[nUpdates][1] = segs[i];
	    nUpdates++;
	}
    }
    for ( i=0; i<nUpdates; i++ ) {
	if ( updates[i][0] == draw )
	    drawSegment( pos, updates[i][1], p, pen );
	if (updates[i][0] == erase)
	    drawSegment( pos, updates[i][1], p, pen, TRUE );
    }
}

void QLCDNumber::drawSegment( const QPoint &pos, int segmentNo, QPainter &p,
			      QPen &pen, bool erase )
{
    QPoint pt = pos;
    QColor lightColor,darkColor;
    if ( erase ){
        lightColor = backgroundColor();
        darkColor  = lightColor;
    }
    else {
        lightColor = backgroundColor().light();
        darkColor  = backgroundColor().dark();
    }
    int yLen = segmentYLength();
    int xLen = segmentXLength();
    if ( xLen > yLen )
        xLen = yLen;
    else
        yLen = xLen;
    int width = xLen*2/9;

#define LINETO(X,Y) p.lineTo(QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT pen.setColor(lightColor)
#define DARK  pen.setColor(darkColor)

    switch ( segmentNo ) {
        case 0 :
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(xLen,0);
	    DARK;
	    LINETO(xLen - width,width);
	    LINETO(width,width);
	    LINETO(0,0);
	    break;
        case 1 :
	    pt.ry()++;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,width);
	    DARK;
	    LINETO(width,yLen - width/2);
	    LINETO(0,yLen);
	    LIGHT;
	    LINETO(0,0);
	    break;
        case 2 :
	    pt.rx() += xLen;
	    pt.ry()++;
	    p.moveTo(pt);
	    DARK;
	    LINETO(0,yLen);
	    LINETO(-width,yLen - width/2);
	    LIGHT;
	    LINETO(-width,width);
	    LINETO(0,0);
	    break;
        case 3 :
	    pt.ry() += yLen + 2;
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
        case 4 :
	    pt.ry() += yLen + 3;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,width/2);
	    DARK;
	    LINETO(width,yLen - width);
	    LINETO(0,yLen);
	    LIGHT;
	    LINETO(0,0);
	    break;
        case 5 :
	    pt.rx() += xLen;
	    pt.ry() += yLen + 3;
	    p.moveTo(pt);
	    DARK;
	    LINETO(0,yLen);
	    LINETO(-width,yLen - width);
	    LIGHT;
	    LINETO(-width,width/2);
	    LINETO(0,0);
	    break;
        case 6 :
	    pt.ry() += 2*yLen + 4;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,-width);
	    LINETO(xLen - width,-width);
	    LINETO(xLen,0);
	    DARK;
	    LINETO(0,0);
	    break;
        case 7 :
	    if ( smallPoint )
		pt.rx() += xLen + xSpace()/2 - width/2 + 1;
	    else
		pt.rx() += xLen/2 + 1;
	    pt.ry() += 2*yLen + 4;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
        case 8 :
	    pt.ry() += yLen/2 + width;
	    pt.rx() += xLen/2 - width/2 + 1;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
        case 9 :
	    pt.ry() += 3*yLen/2 + width;
	    pt.rx() += xLen/2 - width/2 + 1;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
#if defined(CHECK_RANGE)
        default :
	    warning( "QLCDNumber::drawSegment: Internal error."
		     "  Illegal segment id: %s\n", segmentNo );
#endif
    }
    #undef LINETO
    #undef LIGHT
    #undef DARK
}


int QLCDNumber::digitLength() const
{
    return (clientSize().width() - 11)/nDigits;
}

int QLCDNumber::segmentXLength() const
{
    return digitLength() - xSpace();
}

int QLCDNumber::segmentYLength() const
{
    return (clientSize().height() - 16)/2;
}

int QLCDNumber::xSpace() const
{
    if ( smallPoint )
        return digitLength()*4/9;
    else
        return digitLength()/5;
}

int QLCDNumber::xOffset() const
{
    return  (clientSize().width()  -
                (nDigits*(digitLength()) - xSpace()))/2;
}

int QLCDNumber::yOffset() const
{
    return  (clientSize().height() - 2*segmentYLength() - 5)/2;
}
